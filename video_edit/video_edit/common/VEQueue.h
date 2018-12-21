#ifndef __VE_QUEUE_H__
#define __VE_QUEUE_H__

#include <queue>
#include <list>
#include "VESemaphore.h"
#include "TCriticalSection.h"
#include "TConditionVariable.h"


template<class T>
class VEQueue{
public:
	VEQueue();
	~VEQueue();
	void pop(){
		comn::AutoCritSec lock(m_cs);
		m_queue.pop_front();
	}
	void pop_front(){
        comn::AutoCritSec lock(m_cs);
		m_queue.pop_front();
	}
	T& front(){
        comn::AutoCritSec lock(m_cs);
		return m_queue.front();
	}
	void push (const T& val){
        comn::AutoCritSec lock(m_cs);
		m_queue.push_back(val);
	}
	void push_back (const T& val){
        comn::AutoCritSec lock(m_cs);
		m_queue.push_back(val);
	}
    void push_front (const T& val){
        comn::AutoCritSec lock(m_cs);
        m_queue.push_front(val);
    }
    template<class C>
    void sort(C compare){
        comn::AutoCritSec lock(m_cs);
        m_queue.sort(compare);
    }
	size_t size() const{
		return m_queue.size();
	}
	bool empty() const{

		return m_queue.empty();
	}
	void post(){
		m_sem.post();
	}
	void wait(){
		m_sem.wait();
	}
private:
	std::list<T> m_queue;
	VESemaphore m_sem;
	comn::CriticalSection m_cs;
};
template<class T>VEQueue<T>::VEQueue(){}
template<class T>VEQueue<T>::~VEQueue(){}

#endif /* __VE_QUEUE_H__ */
