/********************************************************************
created:	2012/03/13
modify :    2012/03/13
file :      TConditionVariable.h
author:		lvll
purpose:    condition variable
*********************************************************************/
#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H

/////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include "TCriticalSection.h"
#include <time.h>
#include <limits.h>

enum
{
	CVariable_Success = 0,
	CVariable_TimeOut = 1,
	CVariable_Error   = 2
};

#ifndef WIN32
#include <errno.h>
#include <sys/time.h>
#endif




namespace comn
{

#ifdef WIN32
class ConditionVariable
{
public:
	ConditionVariable()
	{
		create();		
	}

	~ConditionVariable()
	 {
		destroy();			
	 }

	bool create()
	{
		m_semaphore = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
		if (m_semaphore == NULL)
		{
			return false;
		}		

		m_waiter = 0;
		m_waker = 0;
		m_generation = 0;
		return true;
	}

	void destroy()
	{
		 CloseHandle(m_semaphore);
	}
	
	int timedwait(CriticalSection &cs,unsigned long millisec)
	{	
		int ok = CVariable_Error;
		DWORD res;
		unsigned int  waker = 0;
		unsigned long generation = 0;

		{
			AutoCritSec  tmpLock(m_cs);
			m_waiter++;
		    generation = m_generation;
		}

		cs.unlock();


		do {
			res = WaitForSingleObject(m_semaphore, millisec);

			{
				AutoCritSec  tmpLock(m_cs);			
				if (m_waker) 
				{
					if (m_generation != generation) 
					{
						m_waker--;
						m_waiter--;	
						ok = CVariable_Success;
						break;
					} 
					else 
					{
						waker = 1;
					}
				}
				else if (res != WAIT_OBJECT_0) 
				{
					m_waiter--;
					ok = CVariable_TimeOut;
					break;
				}
			}			
			
			if (waker) 
			{
				waker = 0;
				ReleaseSemaphore(m_semaphore, 1, NULL);
			}
		} while (1);
		
		cs.lock();

		return ok;
	}


	bool wait(CriticalSection &cs)
	{
		return timedwait(cs,INFINITE) == CVariable_Success;
	}


	bool signal()
	{
		unsigned int wake = 0;

		{
			AutoCritSec  tmpLock(m_cs);
			if (m_waiter > m_waker) 
			{
				wake = 1;
				m_waker++;
				m_generation++;
		    }
		}
		if (wake) 
		{
			ReleaseSemaphore(m_semaphore, 1, NULL);
		}

		return true;			
	}

	bool broadcast()
	{
		unsigned long wake = 0;

		{
			AutoCritSec  tmpLock(m_cs);
			if (m_waiter > m_waker) 
			{
				wake = m_waiter - m_waker;
				m_waker = m_waiter;
				m_generation++;
			}
		}

		if (wake) 
		{
			ReleaseSemaphore(m_semaphore, wake, NULL);
		}
		return true;		
	}

private:	
	 HANDLE m_semaphore;
     CriticalSection m_cs;
     unsigned long m_waiter;
     unsigned long m_waker;
     unsigned long m_generation;
};

#else
/////////////////////////////////////////////////////////////////////////////
///for linux

class ConditionVariable
{
public:
	ConditionVariable()
	{
		create();		
	}

	~ConditionVariable()
	{
		destroy();			
	}

	bool create()
	{	
		return pthread_cond_init(&m_cond, NULL) == 0;
	}

	void destroy()
	{
		pthread_cond_destroy(&m_cond);
	}

	bool wait(CriticalSection &cs)
	{
		return  pthread_cond_wait(&m_cond, &(cs.getLock())) == 0;      
	}

	int timedwait(CriticalSection &cs,unsigned long millisec)
	{			
		struct timespec abstime;
		getTimeout(&abstime,millisec);
		int ret = pthread_cond_timedwait(&m_cond, &(cs.getLock()),(const struct timespec *) &abstime);
		if (ETIMEDOUT == ret) 
		{
			ret = CVariable_TimeOut;
		}
		else if (ret != 0) 
		{
			ret = CVariable_Error;
		}

	    return ret;
	}

	bool signal()
	{
		return  pthread_cond_signal(&m_cond) == 0;				
	}

	bool broadcast()
	{
		return  pthread_cond_broadcast(&m_cond) == 0;				
	}	

	static void getTimeout(struct timespec *abstime, long milliseconds)
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);

		abstime->tv_sec  = tv.tv_sec + milliseconds / 1000;
		abstime->tv_nsec = tv.tv_usec*1000 + (milliseconds % 1000)*1000000;
		if (abstime->tv_nsec >= 1000000000)
		{
			abstime->tv_nsec -= 1000000000;
			abstime->tv_sec++;
		}
	}
private:	
	pthread_cond_t m_cond;
};


#endif //WIN32


} // end of namespace

////////////////////////////////////////////////////////////////////////////
#endif 

