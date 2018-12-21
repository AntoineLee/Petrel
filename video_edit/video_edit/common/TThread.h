

#ifndef TTHREAD_H_
#define TTHREAD_H_

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else //
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/syscall.h>
//#include <unistd.h>
#endif //WIN32
#include <assert.h>
////////////////////////////////////////////////////////////////////////////



namespace comn
{

#ifdef WIN32

class Thread
{
public:
    typedef HANDLE          ThreadHandle;
    typedef unsigned int    ThreadID;

    Thread()
    :m_stackSize(0),m_threadID(0),m_handle(0),m_canExit(false)
    {
    }

    explicit Thread(unsigned int stack_size)
    :m_stackSize(stack_size),m_threadID(0),m_handle(0),m_canExit(false)
    {
    }

    virtual ~Thread()
    {
        assert(isRunning() == false);
    }

    bool start()
    {
        m_canExit = false;
        return createThread();
    }

    void stop()
    {
        tryStop();
        join(-1);
    }

    void tryStop()
    {
        InterlockedExchange((LPLONG)&m_canExit, true);
        doStop();
    }

    bool join(int millisecond = -1)
    {
        bool ok = true;
        if (m_handle != 0)
        {
            DWORD dwMilliseconds = (millisecond >= 0) ? millisecond : INFINITE;
            DWORD res = ::WaitForSingleObject(m_handle, dwMilliseconds);
            ok = (WAIT_OBJECT_0 == res);
        }
        return ok;
    }


    ThreadHandle getHandle() const
    {
        return m_handle;
    }

    ThreadID getThreadID() const
    {
        return m_threadID;
    }

    bool isRunning() const
    {
        return (m_handle != 0);
    }

    void setStackSize(unsigned int stack_size)
    {
        m_stackSize = stack_size;
    }

protected:
    virtual int run() =0;
    virtual bool tstartup() { return true; }
    virtual void tcleanup() {}
    virtual void doStop() {}


    unsigned int    m_stackSize;
    ThreadID        m_threadID;
    ThreadHandle    m_handle;
    bool            m_canExit;



private:
    static unsigned __stdcall ThreadFunction(void* param)
    {
        int code = 0;
        Thread* pThread = (Thread*)param;
        if ( !pThread->m_canExit )
        {
            if (pThread->tstartup())
            {
                code = pThread->run();
            }
            else
            {
                code = 1;
            }
            pThread->tcleanup();
        }
        ::CloseHandle(pThread->m_handle);
        pThread->m_handle = 0;
        pThread->m_threadID = 0;
        _endthreadex(code);
        return code;
    }

    bool createThread()
    {
        unsigned int flag = 0;
        void* param = (void*)this;
        m_handle = (HANDLE)_beginthreadex(NULL, m_stackSize, ThreadFunction,
                            param, flag, &m_threadID);
        return (m_handle != 0);
    }

};

#else //WIN32

class Thread
{
public:
    typedef pthread_t    ThreadHandle;
    typedef pthread_t    ThreadID;

    Thread()
    :m_stackSize(0),m_threadID((pthread_t)0),m_handle((pthread_t)-1),m_canExit(false)
    {
    }

    explicit Thread(unsigned int stack_size)
    :m_stackSize(stack_size),m_threadID((pthread_t)0),m_handle((pthread_t)-1),m_canExit(false)
    {
    }

    virtual ~Thread()
    {
        assert(isRunning() == false);
    }

    bool start()
    {
        m_canExit = false;
        return createThread();
    }

    void stop()
    {
        tryStop();
        join(-1);
    }

    void tryStop()
    {
        m_canExit = true;
        doStop();
    }

    bool join(int millisecond = -1)
    {
		bool ok = true;
		if (m_handle != (pthread_t)-1)
		{
			if (millisecond <= 0)
			{
				ok = (0 == pthread_join(m_handle, NULL));
				m_handle = (pthread_t)-1;
			}
			else
			{
#ifdef _GNU_SOURCE
				struct timespec     ts;
				getTimeout( &ts, millisecond );
				//ok = ( 0 == ::pthread_timedjoin_np(m_handle, NULL, &ts));
#endif
			}
		}

		return ok;       
    }


    ThreadHandle getHandle() const
    {
        return m_handle;
    }

    ThreadID getThreadID() const
    {
        return m_threadID;
    }

    bool isRunning() const
    {
        return (m_handle != (pthread_t)-1);
    }

    void setStackSize(unsigned int stack_size)
    {
        m_stackSize = stack_size;
    }

    int kill(int sig)
    {
        int ret = 0;
        if (isRunning())
        {
            ret = ::pthread_kill(m_handle, sig);
        }
        return ret;
    }

protected:
    virtual int run() =0;
    virtual bool tstartup() { return true; }
    virtual void tcleanup() {}
    virtual void doStop() {}


    unsigned int    m_stackSize;
    ThreadID        m_threadID;
    ThreadHandle    m_handle;
    bool            m_canExit;



private:
    static void* ThreadFunction(void* param)
    {
        int code = 0;
        Thread* pThread = (Thread*)param;
       // pThread->m_threadID = syscall( SYS_gettid );
        if ( !pThread->m_canExit )
        {
            if (pThread->tstartup())
            {
                code = pThread->run();
            }
            else
            {
                code = 1;
            }
            pThread->tcleanup();
        }
       // pThread->m_handle = (pthread_t)-1;
        pThread->m_threadID = 0;
        ::pthread_exit(&code);
        return NULL;
    }

    bool createThread()
    {
        pthread_attr_t attr;
        ::pthread_attr_init( &attr );
        pthread_attr_t* pAttr = NULL;
        if ( 0 != m_stackSize )
        {
            ::pthread_attr_setstacksize( &attr, m_stackSize );
            pAttr = &attr;
        }
       
//        pthread_attr_setschedpolicy(&attr, SCHED_RR);
//        struct  sched_param   param;
//        param.sched_priority = sched_get_priority_max(SCHED_RR);
//        pthread_attr_setschedparam(&attr, &param);
        
        int ret = ::pthread_create( &m_handle, pAttr, ThreadFunction, (void*)this );
       
        
        ::pthread_attr_destroy( &attr );
        return ( 0 == ret );
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

};

#endif //WIN32

} // end of namespace
////////////////////////////////////////////////////////////////////////////

#endif /* TTHREAD_H_ */
