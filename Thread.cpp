#include "Thread.h"

ThreadMsgQueue* Thread::threadMsgPool  = NULL;
int             Thread::maxMsgQueueCnt = 0;

void Thread::init(int maxcnt) {
	maxMsgQueueCnt = maxcnt;
	if (maxMsgQueueCnt < 0) maxMsgQueueCnt = 0;
	threadMsgPool = new ThreadMsgQueue[maxMsgQueueCnt+1];
	bindMsgQueue(0); // 0 means main thread
}

void Thread::create(void *(*start) (void *), void* arg) {
	pthread_t pid;
	pthread_create(&pid, NULL, start, arg);
}

bool Thread::bindMsgQueue(int mqid) {
	if (mqid > maxMsgQueueCnt) return false;
	ThreadMsgQueue* tmq = &threadMsgPool[mqid];
	pthread_mutex_lock(&tmq->mutex);
	if (!pthread_equal(tmq->bindTid, pthread_t())) {
		pthread_mutex_unlock(&tmq->mutex);
		return false;
	}
	tmq->bindTid = pthread_self();
	pthread_mutex_unlock(&tmq->mutex);
	return true;
}

void Thread::unbindMsgQueue(int mqid) {
	if (mqid > maxMsgQueueCnt) return;
	ThreadMsgQueue* tmq = &threadMsgPool[mqid];
	pthread_mutex_lock(&tmq->mutex);
	if (pthread_equal(tmq->bindTid, pthread_self())) {
		tmq->bindTid = pthread_t();
		list<void*>::iterator i;
		for (i = tmq->msgList.begin(); i != tmq->msgList.end(); i++) {
			delete *i; // delete a (void*) pointer will not call the destructor
		}
		tmq->msgList.clear();
	}
	pthread_mutex_unlock(&tmq->mutex);
}

void Thread::send(int mqid, void* obj) {
	if (obj == NULL) return;
	if (mqid > maxMsgQueueCnt) return;
	ThreadMsgQueue* tmq = &threadMsgPool[mqid];
	pthread_mutex_lock(&tmq->mutex);
	if (!pthread_equal(tmq->bindTid, pthread_t())) {
		tmq->msgList.push_back(obj);
	}
	pthread_mutex_unlock(&tmq->mutex);
	pthread_mutex_unlock(&tmq->block);
}

void* Thread::recv(int mqid) {
	if (mqid > maxMsgQueueCnt) return NULL;
	ThreadMsgQueue* tmq = &threadMsgPool[mqid];
	pthread_mutex_lock(&tmq->block);
	pthread_mutex_lock(&tmq->mutex);
	if (!pthread_equal(tmq->bindTid, pthread_self()) || tmq->msgList.empty()) {
		pthread_mutex_unlock(&tmq->mutex);
		return NULL;
	}
	void* ret = tmq->msgList.front();
	tmq->msgList.pop_front();
	if (!tmq->msgList.empty()) {
		pthread_mutex_unlock(&tmq->block);
	}
	pthread_mutex_unlock(&tmq->mutex);
	return ret;
}

ThreadMsgQueue::ThreadMsgQueue() {
	this->msgList = list<void*>();
	this->mutex = PTHREAD_MUTEX_INITIALIZER;
	this->block = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&this->block);
	this->bindTid = pthread_t();
}

ThreadMsgQueue::~ThreadMsgQueue() {
	pthread_mutex_destroy(&this->mutex);
	pthread_mutex_destroy(&this->block);
}