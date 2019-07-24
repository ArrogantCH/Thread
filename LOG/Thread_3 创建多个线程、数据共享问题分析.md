## Thread_3 创建多个线程、数据共享问题分析

### 一 创建和等待多个线程

创建10个线程，线程入口函数统一使用 myprint
	 a) 多个线程执行顺序是乱的，跟操作系统内部对线程的运行调度机制有关
	 b) 主线程等待所有子线程运行结束，最后主线程结束，推荐join(), 更容易写出稳定的程序
	 c) 咱们把thread对象放入到容器里管理，看起来像个thread数组，这样管理很方便

```c++
//线程入口函数
void myprint(int num)
{
	std::cout << "myprint线程开始执行了，线程编号=" << num << std::endl;
	//......干各种事情
	std::cout << "myprint线程结束执行了，线程编号=" << num << std::endl;
	return;
}

int main()
{
	std::vector<std::thread> mythreads;
	for (int i = 0; i < 10; i++)
	{
		mythreads.push_back(std::thread(myprint, i)); //创建10个线程，同时这10个线程已经开始执行
	}
	for (auto iter = mythreads.begin(); iter != mythreads.end(); iter++)
	{
		iter->join(); //等待10个线程都返回
	}
	std::cout << "I love China" << std::endl;
}
```

### 二 数据共享问题分析

#### 2.1 只读的数据

是安全稳定的，不需要特别的什么处理手段，直接读就可以

#### 2.2 有读有写

 2个线程写，8个线程读。 如果代码没有特别处理，程序肯定崩溃， 最简单的处理，读的时候不能写，写的时候不能读。2个线程不能同时写，8个线程不能同时读

####  2,3 其他案例     

数据共享北京-深圳 火车， 10个售票窗口 卖票     1，2窗口都要订99号座位

### 三 共享数据的保护案例代码（此处没有实现互斥）

网络游戏服务器。 两个自己创建的线程，一个线程收集玩家命令（用一个数字代表玩家发的命令），并把命令数据写到一个队列中，另外一个线程从队列中取出玩家发送来的命令，解析，然后执行玩家需要的动作

准备用成员函数作为线程函数方法
代码化解决问题： 引入一个c++解决多线程保护共享数据问题的第一个概念“互斥量”

```c++
class A
{
public:
	//把收到的消息（玩家命令） 入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
			msgRecvQueue.push_back(i);
		}
	}

	//把收到的消息（玩家命令） 取出的线程
	void outMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			if (!msgRecvQueue.empty())
			{
				//消息不为空
				int command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在
				msgRecvQueue.pop_front(); //移除第一个元素，但不返回
			}
			else
			{
				std::cout << "outMsgRecvQueue执行，但目前消息队列为空" << i << std::endl;
			}
		}
		std::cout << "end";
	}

private:
	std::list<int> msgRecvQueue; //容器，用于命令列表

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

