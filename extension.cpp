/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

ThinkHooks g_ThinkHooks;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_ThinkHooks);

class ForwardNativeHelpers :
	public IHandleTypeDispatch
{
public:
	void OnSourceModAllInitialized()
	{
		HandleAccess sec;

		/* Set GlobalFwd handle access security */
		handlesys->InitAccessDefaults(NULL, &sec);
		sec.access[HandleAccess_Read] = 0;
		sec.access[HandleAccess_Clone] = HANDLE_RESTRICT_IDENTITY | HANDLE_RESTRICT_OWNER;

		/* Create 'GlobalFwd' handle type */
		GlobalFwdType = handlesys->CreateType("", this, 0, NULL, &sec, myself->GetIdentity(), NULL);

		/* Private forwards are cloneable */
		sec.access[HandleAccess_Clone] = 0;

		/* Create 'PrivateFwd' handle type */
		PrivateFwdType = handlesys->CreateType("", this, GlobalFwdType, NULL, &sec, myself->GetIdentity(), NULL);
	}

	void OnSourceModShutdown()
	{
		handlesys->RemoveType(PrivateFwdType, myself->GetIdentity());
		handlesys->RemoveType(GlobalFwdType, myself->GetIdentity());
	}

	void OnHandleDestroy(HandleType_t type, void *object)
	{
		IForward *pForward = static_cast<IForward *>(object);

		forwards->ReleaseForward(pForward);
	}

	bool GetHandleApproxSize(HandleType_t type, void *object, unsigned int *pSize)
	{
		*pSize = sizeof(IForward*) + (((IForward *)object)->GetFunctionCount() * 12);
		return true;
	}

	HandleType_t GlobalFwdType;
	HandleType_t PrivateFwdType;
} g_ForwardNativeHelpers;

const sp_nativeinfo_t MyNatives[] =
{
	{ "RequestThink",	sm_AddThinkAction },
	{ NULL,			NULL },
};

//Let's declare a hook to IServerGameDLL::Think
SH_DECL_HOOK1_void(IServerGameDLL, Think, SH_NOATTRIB, 0, bool);

bool ThinkHooks::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late)
{
	//Let's hook IServerGameDLL::Think
	SH_ADD_HOOK(IServerGameDLL, Think, gamedll, SH_MEMBER(this, &ThinkHooks::Think), false);
	return true;
}

void ThinkHooks::SDK_OnAllLoaded()
{
	sharesys->AddNatives(myself, MyNatives);
	//let's instantiate the action buffer
	m_ActionBuffer = new CActionBuffer<Queue<Action_t>, MutexWrapper>();
	//and the forward helper
	g_ForwardNativeHelpers.OnSourceModAllInitialized();

	//creating the global OnServerThink forward
	m_pOnServerThink = forwards->CreateForward("OnServerThink", ET_Ignore, 0, NULL);
	sharesys->RegisterLibrary(myself, LIB_NAME);
	m_bReady = true;
}

void ThinkHooks::SDK_OnUnload()
{
	//releasing the OnServerThink forward
	forwards->ReleaseForward(m_pOnServerThink);

	//killing the forward helper
	g_ForwardNativeHelpers.OnSourceModShutdown();
	delete m_ActionBuffer;
}

bool ThinkHooks::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	//unhooking IServerGameDLL::Think
	SH_REMOVE_HOOK(IServerGameDLL, Think, gamedll, SH_MEMBER(this, &ThinkHooks::Think), true);
	return true;
}

void ThinkHooks::Think(bool b_final_tick)
{
	if (m_bReady)
	{
		m_ActionBuffer->RunActions();

		if (m_pOnServerThink->GetFunctionCount())
		{
			m_pOnServerThink->Execute();
		}
	}
}

static void ExecPawnActionFromActionData(void *pData)
{
	//casting to ``ActionData_t`` from pData
	ke::AutoPtr<ActionData_t> action(reinterpret_cast<ActionData_t *>(pData));
	//getting the plugin interface from the ownerhandle
	IPlugin *pPlugin = plsys->PluginFromHandle(action->ownerhandle, NULL);
	//if null, the plugin probably failed to load or something, so we return
	if (!pPlugin)
	{
		return;
	}
	
	IChangeableForward *pForward;
	HandleSecurity sec(pPlugin->GetIdentity(), myself->GetIdentity());
	//if we can't read from the handle, we return
	if (handlesys->ReadHandle(action->handle, g_ForwardNativeHelpers.PrivateFwdType, &sec, (void **)&pForward) != HandleError_None)
	{
		return;
	}

	//running the forward
	pForward->PushCell(action->data);
	pForward->Execute(NULL);

	handlesys->FreeHandle(action->handle, &sec);
}

static cell_t sm_AddThinkAction(IPluginContext *pContext, const cell_t *params)
{
	IPluginFunction *pFunction = pContext->GetFunctionById(params[1]);
	IPlugin *pPlugin = plsys->FindPluginByContext(pContext->GetContext());
	if (!pFunction)
	{
		return pContext->ThrowNativeError("Invalid function id (%X)", params[1]);
	}

	IChangeableForward *pForward = forwards->CreateForwardEx(NULL, ET_Ignore, 1, NULL, Param_Cell);
	IdentityToken_t *pIdentity = pContext->GetIdentity();
	Handle_t Handle = handlesys->CreateHandle(g_ForwardNativeHelpers.PrivateFwdType, pForward, pIdentity, myself->GetIdentity(), NULL);
	if (Handle == BAD_HANDLE)
	{
		forwards->ReleaseForward(pForward);
		return 0;
	}

	pForward->AddFunction(pFunction);

	ActionData_t *pData = new ActionData_t(Handle, pPlugin->GetMyHandle(), params[2]);
	g_ThinkHooks.m_ActionBuffer->AddAction(Action_t(ExecPawnActionFromActionData, pData));
	return 1;
}
