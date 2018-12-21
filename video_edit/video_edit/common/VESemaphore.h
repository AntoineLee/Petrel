#ifndef __VE_SEMAPHORE_H__
#define __VE_SEMAPHORE_H__

#include <pthread.h>
#include "VELock.h"

class VESemaphore{

public:
	/**
	  * @brief  信号量构造函数
	  * @param[in]  value 信号量的初始值，大于等于0
	*/
	VESemaphore(unsigned int value = 0);

	/**
	  * @brief  信号量析构函数
	*/
	~VESemaphore();

	/**
	  * @brief  信号量UP操作
	  * @return void
	*/
	void post();
	/**
	  * @brief  信号量Down操作,阻塞
	  * @return void
	*/
	void wait();
	/**
	  * @brief  信号量Down操作，非阻塞
	  * @return decrement成功返回0，否则返回-1
	*/
	int tryWait();

private:

	/**
	  * @brief	锁
	*/
	VELock	m_lock;
	/**
	  * @brief	条件变量
	*/
	pthread_cond_t	m_cond;
	/**
	  * @brief	信号量当前的值
	*/
	unsigned int	m_value;
};
#endif /*__VE_SEMAPHORE_H__*/
