# Thread_5 unique_lock详解

## 一 unique_lock取代lock_guard

unique_lock是个类模版，工作中一般lock_guard(推荐使用)

unique_lock比lock_guard灵活很多，效率上差一点，内存占用多一点

## 二 unique_lock的的第二个参数

### 2.1 std::adopt_lock

表示这个互斥量已经被lock了（你必须要把这个互斥量提前lock了，否则会报异常）

```c++
void inMsgRecvQueue()
{
    for (int i = 0; i < 100000; i++)
    {
        std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
        my_mutex.lock();
        std::unique_lock<std::mutex> sguard(my_mutex, std::adopt_lock);
        msgRecvQueue.push_back(i);
    }
}
```

### 2.2 std::try_to_lock

我们会尝试用mutex的lock()去锁定这个mutex，但如果没有锁定成功，我也会立即返回，并不会阻塞在那里, 可以做一些别的事情

用这个try_to_lock的前提是你自己不能先去lock

```c++
void inMsgRecvQueue()
{
    for (int i = 0; i < 100000; i++)
    {
        std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
        std::unique_lock<std::mutex> sguard(my_mutex, std::try_to_lock);
        if (sguard.owns_lock())
        {
            //拿到了锁头
            msgRecvQueue.push_back(i);
        }
        else
        {
            //没拿到锁
            std::cout << "没有拿到锁头，干点别的事儿" << i << std::endl;
        }		
        msgRecvQueue.push_back(i);

    }
}
```

### 2.3 std::defer_lock  需要配合unique_lock 成员函数使用

用这个defer_lock的前提： 你不能自己先lock，否则会报异常

defer_lock的意思：并没有给mutex加锁，初始化一个没有加锁的mutex

## 三 unique_lock成员函数

### 3.1 lock() 加锁

```c++
void inMsgRecvQueue()
{
    for (int i = 0; i < 100000; i++)
    {
        std::unique_lock<std::mutex> sguard(my_mutex, std::defer_lock); //没加锁的my_mutex
        sguard.lock(); //不用自己unlock，如果要有自己处理的非共享代码，就手动unlock		
        msgRecvQueue.push_back(i);
    }
}
```

### 3.2 unlock() 解锁

```c++
void inMsgRecvQueue()
{
    for (int i = 0; i < 100000; i++)
    {
        std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
        std::unique_lock<std::mutex> sguard(my_mutex, std::defer_lock); //没加锁的my_mutex
        sguard.lock(); //不用自己unlock，如果要有自己处理的非共享代码，就手动unlock
        //处理共享代码
        sguard.unlock();
        //处理一些非共享代码
        sguard.lock(); //此处的lock会自动unlock()
        //处理共享代码
        msgRecvQueue.push_back(i);
    }
}
```

### 3.3 try_lock()

尝试给互斥量加锁，如果拿不到锁，则返回false，如果拿到了锁，返回true，这个函数是不阻塞的

```c++
void inMsgRecvQueue()
	{
		for (int i = 0; i < 100000; i++)
		{
			std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
			std::unique_lock<std::mutex> sguard(my_mutex, std::defer_lock); //没加锁的my_mutex
			if (sguard.try_lock() == true) //返回true表示拿到锁了
			{
				msgRecvQueue.push_back(i);
			}
			else
			{
				std::cout << "没有拿到锁头，干点别的事儿" << i << std::endl;
			}
		}
	}
```

### 3.4 release()

返回它所管理的mutex对象指针，并释放所有权；也就是说，这个unique_lock和mutex不再有关系

严格区分unlock()和resleas()的区别，不要混淆

如果原来mutex对象处于加锁状态，你有责任接管过来并负责解锁

```c++
void inMsgRecvQueue()
{
    for (int i = 0; i < 100000; i++)
    {
        std::cout << "inMsgRecvQueue执行，插入一个元素" << i << std::endl;
        std::unique_lock<std::mutex> sguard(my_mutex);
        std::mutex *ptx = sguard.release(); //用ptx接管my_mutex，现在你有责任自己解锁这个my_mutex了
        msgRecvQueue.push_back(i);
        ptx->unlock(); //自己负责my_mutex的unlock
    }
}
```



### 经验：

为什么有时候需要unlock(),因为你lock锁住的代码越少，执行越快，整个程序运行效率越高

有人也把锁头锁住的代码多少 称为锁的粒度，粒度一般用粗细来描述

a)锁住的代码少，这个粒度叫细，执行效率高

b)锁住的代码高，这个粒度叫粗，执行效率低

粒度较细，可能漏掉共享数据的保护；粒度太粗，影响效率



## 四 unique_lock所有权的传递 mutex

```c++
std::unique_lock<std::mutex> 
```

sguard(my_mutex); sgurad拥有my_mutex的所有权

sguard可以把对my_mutex的所有权转移给其他的unique_lock对象

所以，unique_lock对象这个mutex的所有权是属于  可以转移，但是不能复制

### 4.1 std::move

```c++
std::unique_lock<std::mutex> sguard(my_mutex); 
std::unique_lock<std::mutex> sguard2(std::move(sguard)); //移动语义，相当于sguard2和my_mutex绑定到一起了	
//现在sguard指向空， sguard指向my_mutex
```

### 4.2 return std::unique::lock(std::move)

```c++
std::unique_lock<std::mutex> rtn_unique_lock()
{
    std::unique_lock<std::mutex> temguard(my_mutex);
    return temguard; //从函数返回一个局部的unique_lock对象是可以的。 
    //返回这种局部对象temguard会导致系统生成临时unique_lock对象，并调用unique_lock的移动构造函数
}

std::unique_lock<std::mutex> sguard3 = rtn_unique_lock();
```

