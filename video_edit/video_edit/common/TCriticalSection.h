

#if !defined TCRITICALSECTION__
#define TCRITICALSECTION__

////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <windows.h>
#else //
#include <pthread.h>
#endif //WIN32

/////////////////////////////////////////////////////////////////////////////
namespace comn
{
/// cross platform critical section
class CriticalSection
{
public:
    CriticalSection()
    {
        ::pthread_mutex_init(&m_mutex, /*&attr*/NULL);
    }

    ~CriticalSection()
    {
        ::pthread_mutex_destroy(&m_mutex);
    }

    void lock()
    {
        ::pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        ::pthread_mutex_unlock(&m_mutex);
    }

    bool trylock()
    {
        return (0 == ::pthread_mutex_trylock(&m_mutex));
    }

	pthread_mutex_t &getLock()
	{
		return m_mutex;
	}

private:
    pthread_mutex_t m_mutex;

};


/// auto critical section
class AutoCritSec
{
public:
    explicit AutoCritSec(CriticalSection& cs) :
        m_locked(false), m_cs(cs)
    {
        m_cs.lock();
        m_locked = true;
    }

    ~AutoCritSec()
    {
        unlock();
    }

    void unlock()
    {
        if (m_locked){
            m_cs.unlock();
            m_locked = false;
        }
    }

private:
    bool m_locked;
    CriticalSection& m_cs;

    AutoCritSec(const AutoCritSec& mtx);
    AutoCritSec& operator =(const AutoCritSec& mtx);

};

} // end of namespace
////////////////////////////////////////////////////////////////////////////
#endif //TCRITICALSECTION__
