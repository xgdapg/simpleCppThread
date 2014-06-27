Usage
===============

进行所有操作之前先初始化线程消息池 `Thread::init();`
定义线程主体函数 `static void* thread_funcation(void *arg);` // 必须为static
函数主体：
//一次性线程：
```
void* thread_funcation(void *arg) {
	//do something;
	return 0;
}
```
//循环接收消息线程：
```
void* thread_funcation(void *arg) {
	int mqid = 1; // mqid in [1, maxMsgQueueCnt]
	if (Thread::bindMsgQueue(mqid)) {
		do {
			CCString* str = (CCString*)Thread::recv(mqid);
			if (str == NULL) break;
			//do something;
			CCLOG(str->getCString());
			delete str;
		} while(1);
		Thread::unbindMsgQueue(mqid);
	}
	return 0;
}
```
//创建线程
```
Thread::create(thread_funcation);
```
//向其它进程发消息：
```
Thread::send(1, new CCString("OK")); //mqid为接收消息的线程绑定的消息队列ID
```
注意：！！！
发送的消息【一定不能】是autorelease过的！要直接new出来传递！
