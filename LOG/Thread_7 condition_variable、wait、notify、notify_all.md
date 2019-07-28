# Thread_7 condition_variable、wait、notify、notify_all

## 一 条件变量 std::condition_variable、wait()、notify_one()

**notify_one()**：只能通知一个线程，唤醒在当前unique_lock上等待等单个线程，果所有线程都在此对象上等待，则会选择唤醒其中一个线程。选择是任意性的

**std::condition_variable**实际上是一个类，是一个和条件相关等一个类，说白了就是等待一个条件达成，这个类是需要和互斥量来配合工作，用的时候我们要生成这个类的对象



```c++
class A
{
public:

	//把收到的消息（玩家命令） 入到一个队列的线程
	void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::unique_lock<std::mutex> sguard(my_mutex);
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
			msgRecvQueue.push_back(i);

			//假如outMsgRecvQueue()正在处理一个事务，需要一段时间，而不是正卡住wait()那里等待你唤醒，那么此时notify_one也许就没有效果
			my_cond.notify_one(); //我们尝试把wait()的线程唤醒，执行完这行，那么outMsgRecvQueue()里面等wait就会被唤醒
		}
	}

	void outMsgRecvQueue()
	{
		int command = 0;
		while (true)
		{
			std::unique_lock<std::mutex> sguard(my_mutex);
			my_cond.wait(sguard, [this] {   // 一个lambda就是一个可调用对象(函数)
				if (!msgRecvQueue.empty())
					return true;
				return false;
			});
			//流程只要能走到这里来，这个互斥锁一定是锁着的, 同时msgRecvQueue至少是有一条数据的
			// 一会儿再写其他的
			command = msgRecvQueue.front(); //返回第一个元素，但不检查元素是否存在 
			msgRecvQueue.pop_front();
			std::cout << "outMsgRecvQueue执行，取出一个元素:" << command << " thread_id:" << std::this_thread::get_id() << std::endl;
			sguard.unlock(); //因为unique的灵活性，所以我们可以随时等unlock解锁，以免锁住太长时间
			

			// 执行一些具体动作，帮助玩家抽卡，抽卡需要100毫秒等处理时间
			// ...
			// 执行100ms
			//

		}
	}

private:
	std::list<int> msgRecvQueue; //容器，用于命令列表
	std::mutex my_mutex; //创建了一个互斥量,一把锁
	std::condition_variable my_cond; //生成一个条件变量对象

};
```

```c++
std::condition_variable my_cond; //生成一个条件变量对象
```

```c++
std::unique_lock<std::mutex> sguard(my_mutex);
my_cond.wait(sguard, [this] {   // 一个lambda就是一个可调用对象(函数)
    if (!msgRecvQueue.empty())
        return true;
    return false;
});
```

### wait()  用来等一个东西

如果第二个参数lambda表达式返回值是true，那wait()直接返回

如果第二个参数lambda表达式返回值是false, 那么wait()将解锁互斥量，并阻塞到本行

**那阻塞到什么时候为止呢？**

阻塞到其他某个线程调用notify_one()成员函数为止

如果wait()没有第二个参数：my_cond.wait(sguard):那么就跟第二个参数lambda表达式返回false效果一样。那么wait()将解锁互斥量，并阻塞到本行, 阻塞到其他某个线程调用notify_one()成员函数为止

**当其他线程用notify_one() 将本wait()唤醒后，wait()就开始恢复干活了，恢复后干什么活？**

(a) wait()不断尝试重新获取互斥量锁，如果获取不到，那么流程就卡在wait这里等着获取，如果获取到了，那么wait就继续执行(b)

(b)上锁(实际上获取到了锁，就等于上了锁)

b.1 如果wait有第二个参数（lambda），就判断这个lambda表达式，如果lambda表达式为false, 那么wait又对互斥量解锁，然后又休眠这里等待再次被notify_one唤醒

b.2 如果lambda表达式为true，则wait返回，流程走下来（此时互斥锁被锁住）

b.3 如果wait没有第二个参数，则wait返回，流程走下来



## 二 上述代码深入思考

上述代码在执行的时候，很显然由于线程争夺锁的随机性，共享队列中可能就会存在很多数据，这样就可能造成队列内数据的堆积。而从队列中取数据的线程，一次只取一条，这样的效率会不会是个问题？如何解决，要不要解决？

并且，在编写多线程程序的时候，一定要会用，用好，这样才能写出没有bug的程序，不然，程序可能跑一段时间就莫名其妙的崩溃，所以一定要设计好



## 三 notify_all()

唤醒在当前unique_lock上等待的所有线程，所有线程同时竞争哦

