/*
**
*/
#if defined _think_hooks_included
 #endinput
#endif
#define _think_hooks_included

/**
 * Called before every server think. Note that you should avoid
 * doing expensive computations or declaring large local arrays.
 *
 */
forward void OnServerThink();

/**
 * Defines a RequestThink Callback.
 *
 * @param a_data			Data passed to the RequestThink native.
 */
typedef RequestThinkCallback = function void (any a_data);

/**
 * Creates a single use Next Think hook.
 *
 * @param f_function	Function to call on the next think call.
 * @param a_data		Value to be passed on the invocation of the Function.
 */
native void RequestThink(RequestThinkCallback f_function, any a_data = 0);

#if !defined REQUIRE_EXTENSIONS
public __ext_think_hooks_SetNTVOptional()
{
	MarkNativeAsOptional("RequestThink");
}
#endif

public Extension __ext_think_hooks = 
{
	name = "think_hooks",
	file = "think_hooks.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};
