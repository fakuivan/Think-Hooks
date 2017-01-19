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
