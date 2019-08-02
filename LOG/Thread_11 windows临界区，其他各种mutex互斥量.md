# Thread_11 windows临界区，其他各种mutex互斥量

## 一 windows临界区

```c++
InitializeCriticalSection(&my_winsec);//用临界区之前要先初始化
EnterCriticalSection(&my_winsec); //进入临界区
msgRecvQueue.push_back(i);
LeaveCriticalSection(&my_winsec);//离开临界区
```



```c++
#include <Windows.h>

#define _WINDOWSJQ_
class A
{
public:
    A()
	{
#ifdef _WINDOWSJQ_
		InitializeCriticalSection(&my_winsec); //用临界区之前要先初始化
#endif
	}
    
	//把收到的消息（玩家命令） 入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
#ifdef _WINDOWSJQ_
			EnterCriticalSection(&my_winsec); //用临界区之前要先初始化
			msgRecvQueue.push_back(i);
			LeaveCriticalSection(&my_winsec);
#endif
		}
	}



	bool outMsgLULProc(int &command)
	{
#ifdef _WINDOWSJQ_
		EnterCriticalSection(&my_winsec); //用临界区之前要先初始化
		if (!msgRecvQueue.empty())
		{
			//消息不为空
			command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在 
			msgRecvQueue.pop_front(); //移除第一个元素，但不返回
			my_mutex.unlock();
			return true;
		}
		LeaveCriticalSection(&my_winsec);
#endif
		return false;
		
	}

	//把收到的消息（玩家命令） 取出的线程
	void outMsgRecvQueue()
	{
		int command = 0;
		for (int i = 0; i < 100000; i++)
		{
			bool result = outMsgLULProc(command);
			if (result)
			{
				std::cout << "outMsgRecvQueue执行，取出一个元素:" << command << std::endl;
				//可以考虑进行命令(数据)处理
			}
			else
			{
				std::cout << "outMsgRecvQueue执行，但目前消息队列被锁" << i << std::endl;
			}
		}
		std::cout << "end";
	}



private:
#ifdef _WINDOWSJQ_
	CRITICAL_SECTION my_winsec; //windows中的临界区，非常类似于c++中的mutex
#endif

};

int main()
{
	A myobja;
	std::thread myOutnMsgobj(&A::outMsgRecvQueue, &myobja); //第二个参数是 引用，才能保证线程理用的是同一个对象
	std::thread myInMsgobj(&A::inMsgRecvQueue, &myobja);

	myInMsgobj.join();
	myOutnMsgobj.join();

}

```



## 二 多次进入临界区试验

在同一个线程(不同线程就会卡住等待)中，windows中的“相同临界区变量”代表的临界区的进入(EnterCriticalSection)可以被多次调用

但是调用了几次EnterCriticalSection，就得调用几次LeaveCriticalSection

```c++
void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			EnterCriticalSection(&my_winsec); 
            EnterCriticalSection(&my_winsec); //多次进入临界区
			msgRecvQueue.push_back(i);
			LeaveCriticalSection(&my_winsec);
            LeaveCriticalSection(&my_winsec);

		}
	}
```

 c++11 中的mutex不允许同一个线程中lock同一个互斥量多次，会报异常

```c++
void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			
			my_mutex.lock();
			my_mutex.lock(); //报异常，和windows有区别
			msgRecvQueue.push_back(i);
			my_mutex.unlock();
			my_mutex.unlock();

		}
	}
```



## 三 自动析构技术

在c++11 中，类似于std::lock_guard

其实，我们可以自己编写一个类用于windows临界区的自动析构

```c++
//本类用于自动释放windows下的临界区，防止忘记LeaveCriticalSection导致死锁，
class CWinLock  //叫RAII类(Resource Acquisition is initialization) 资源获取及初始化
				//容器，智能指针等等
{
public:
	CWinLock(CRITICAL_SECTION *m_pCriticaltmp)  //构造函数
	{
		m_pCritical = m_pCriticaltmp;
		EnterCriticalSection(m_pCritical);
	}

	~CWinLock()  //析构函数
	{
		LeaveCriticalSection(m_pCritical);
	}

private:
	CRITICAL_SECTION *m_pCritical;

};
```

```c++
void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
#ifdef _WINDOWSJQ_
			CWinLock wlock(&my_winsec);
			CWinLock wlock2(&my_winsec); //windows连续两次没问题
			msgRecvQueue.push_back(i);
#else

			std::lock_guard<std::mutex> sbguard(my_mutex);
			msgRecvQueue.push_back(i);
#endif
		}
	}
```

