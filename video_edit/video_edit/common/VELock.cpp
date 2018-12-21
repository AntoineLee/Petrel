#include "VELog.h"
#include "VELock.h"

VELock::VELock(VE_LOCK_TYPE type):m_type(type){

	//初始化锁属性
	pthread_mutexattr_t		attr;

    pthread_mutexattr_init(&attr);
    /*
	if(pthread_mutexattr_init(&attr)){
		VE_LOG_ERROR("VELock::VELock pthread_mutexattr_init failed");
	}
     */
	//初始化锁类型
    pthread_mutexattr_settype(&attr,m_type);
    /*
	if(pthread_mutexattr_settype(&attr,m_type)){
		VE_LOG_ERROR("VELock::VELock pthread_mutexattr_settype failed");
	}
     */

	//初始化锁
    pthread_mutex_init(&m_lock,&attr);
    /*
    if(pthread_mutex_init(&m_lock,&attr)){
		VE_LOG_ERROR("VELock::VELock pthread_mutex_init failed");
	}
     */
	//释放属性对象
    pthread_mutexattr_destroy(&attr);
    /*
	if(pthread_mutexattr_destroy(&attr)){
		VE_LOG_ERROR("VELock::VELock pthread_mutexattr_destroy failed");
	}
     */

}
pthread_mutex_t* VELock::getLock(){return &m_lock;}
VELock::~VELock(){

	//释放属性对象
    pthread_mutex_destroy(&m_lock);
    /*
	if(pthread_mutex_destroy(&m_lock)){
		VE_LOG_ERROR("VELock::VELock pthread_mutex_destroy failed");
	}
     */

}
void VELock::lock(){

	//阻塞上锁
    pthread_mutex_lock(&m_lock);
    /*
	if(pthread_mutex_lock(&m_lock)){
		VE_LOG_ERROR("VELock::VELock pthread_mutex_lock failed,m_type=%d",m_type);
	}
     */
}
void VELock::trylock(){
	//非阻塞上锁
    pthread_mutex_trylock(&m_lock);
    /*
	if(pthread_mutex_trylock(&m_lock)){
		VE_LOG_ERROR("VELock::VELock pthread_mutex_trylock failed,m_type=%d",m_type);
	}
     */
}
void VELock::unlock(){
	//解锁
    pthread_mutex_unlock(&m_lock);
    /*
	if(pthread_mutex_unlock(&m_lock)){
		VE_LOG_ERROR("VELock::VELock pthread_mutex_unlock failed,m_type=%d",m_type);
	}
     */
}
