#ifndef __VE_LOCK_H__
#define __VE_LOCK_H__

#include <pthread.h>

typedef enum{
	VE_LOCK_TYPE_NORMAL = PTHREAD_MUTEX_NORMAL,
	VE_LOCK_TYPE_RECURSIVE = PTHREAD_MUTEX_RECURSIVE,
	VE_LOCK_TYPE_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK,
}VE_LOCK_TYPE;

/**
  * @class  VELock VELock.h
  * @brief  线程锁类

*/
class VELock{
public:
	/**
	  * @brief  构造函数
	  * @param[in]  type 锁类型
	*/
	VELock(VE_LOCK_TYPE type);
	/**
	  * @brief  析构函数

	*/
	~VELock();
	/**
	  * @brief  上锁
	  * @return void
	*/
	void lock();
	/**
	  * @brief  lock的非阻塞版本
	  * @return void
	*/
	void trylock();
	/**
	  * @brief  解锁
	  * @return void
	*/
	void unlock();

	/**
	  * @brief  获得所对象
	  * @return void*	mutex所对象
	*/
	pthread_mutex_t* getLock();


private:
	/**
	  * @brief  锁的类型
	*/
	VE_LOCK_TYPE	m_type;
	/**
	  * @brief  mutex锁
	*/
	pthread_mutex_t	m_lock;
};

class VEAutoLock{
public:
	VEAutoLock(VELock &lock):m_lock(lock){
		m_lock.lock();
	}
	~VEAutoLock(){
		m_lock.unlock();
	}
private:
	VELock	&m_lock;
};

#endif /* __VE_LOCK_H__ */
