#pragma once

/**
* @brief A typedef that defines the function to be called on CActionBuffer::RunActions.
*
*/
typedef void(*ACTION)(void *data);

/**
* @brief A struct that defines an object to be passed to the CActionBuffer.
*
*/
struct Action_t
{
	Action_t(ACTION a, void *d) : data(d), action(a) {  };

	/**
	* @brief	A pointer to the data to be passed when calling action.
	*/
	void *data;

	/**
	* @brief	A function to be called.
	*/
	ACTION action;
};

template<class Q, class M>
class CActionBuffer
{
public:
	CActionBuffer();
	void RunActions();
	void AddAction(const Action_t & action);
	~CActionBuffer();
private:
	M *m_pMutex;
	Q *m_pQueue;
	Q *m_pActions;
};

template<class Q, class M>
CActionBuffer<Q, M>::CActionBuffer()
{
	m_pQueue = new Q();
	m_pActions = new Q();
	m_pMutex = new M();
}

template<class Q, class M>
void CActionBuffer<Q, M>::RunActions()
{
	/* Now we process NextThinkCall hooks. It's okay if this check races. */
	if (m_pQueue->size())
	{
		Q *temp;

		/* Very quick lock to move m_pQueue/m_pActions back and forth */
		m_pMutex->Lock();

		temp = m_pQueue;
		m_pQueue = m_pActions;
		m_pActions = temp;

		m_pMutex->Unlock();

		/* The server will now be adding to the other queue, so we can process events. */
		while (!m_pActions->empty())
		{
			Action_t &item = m_pActions->first();
			m_pActions->pop();
			item.action(item.data);
		}
	}
}

template<class Q, class M>
void CActionBuffer<Q, M>::AddAction(const Action_t & action)
{
	m_pMutex->Lock();
	m_pQueue->push(action);
	m_pMutex->Unlock();
}

template<class Q, class M>
CActionBuffer<Q, M>::~CActionBuffer()
{
	delete m_pQueue;
	delete m_pActions;
	delete m_pMutex;
}