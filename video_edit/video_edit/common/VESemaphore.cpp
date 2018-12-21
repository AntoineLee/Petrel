#include "VESemaphore.h"
#include "VELog.h"

VESemaphore::VESemaphore(unsigned int value):m_value(value),m_lock(VE_LOCK_TYPE_NORMAL){

	//初始化条件变量
    pthread_cond_init(&m_cond,NULL);
    /*
    if(pthread_cond_init(&m_cond,NULL)){
		VE_LOG_ERROR("VESemaphore::VESemaphore:pthread_cond_init failed");
	}
     */

}

VESemaphore::~VESemaphore(){

	//销毁USemaphore
    pthread_cond_destroy(&m_cond);
    /*
	if(pthread_cond_destroy(&m_cond)){
		VE_LOG_ERROR("VESemaphore::~VESemaphore:pthread_cond_destroy failed");
	}
     */
}

void VESemaphore::wait(){

	VEAutoLock lock(m_lock);

	while( 0 == m_value ){
		//等待
        pthread_cond_wait(&m_cond,m_lock.getLock());
		/*
        if(pthread_cond_wait(&m_cond,m_lock.getLock())){
			VE_LOG_ERROR("VESemaphore::wait:pthread_cond_wait failed");
		}
         */
	}
	m_value--;
}
void VESemaphore::post(){

	VEAutoLock lock(m_lock);

	m_value++;

    pthread_cond_signal(&m_cond);
    /*
	if(pthread_cond_signal(&m_cond)){
		VE_LOG_ERROR("VESemaphore::posts:pthread_cond_signal failed");
	}
     */
}

int VESemaphore::tryWait(){

	VEAutoLock lock(m_lock);

	if( 0 == m_value )return -1;

	m_value--;

	return 0;
}
