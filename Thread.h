#pragma once
#include "pthread.h"
#include <list>
using std::list;

class ThreadMsgQueue;
class Thread
{
public:
	static void  init(int maxMsgQueueCnt = 0);
	static void  create(void *(*start) (void *), void* arg = NULL);
				 
	static void  send(int mqid, void* obj);
	static void* recv(int mqid);
	static bool  bindMsgQueue(int mqid);
	static void  unbindMsgQueue(int mqid);

private:
	static ThreadMsgQueue* threadMsgPool;
	static int             maxMsgQueueCnt;
};

class ThreadMsgQueue
{
public:
	ThreadMsgQueue();
	~ThreadMsgQueue();

	list<void*>     msgList;
	pthread_mutex_t mutex;
	pthread_mutex_t block;
	pthread_t       bindTid;
};
