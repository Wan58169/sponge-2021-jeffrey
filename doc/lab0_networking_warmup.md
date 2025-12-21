# 1-PreAction

在进行lab0分支后，进入sponge目录，创建build文件夹，在build文件夹里执行cmake和make，

```bash
cd sponge-2021-jeffrey
mkdir build && cd build
# 打开Debug选项
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
```

在cmake中启用Debug后，就可以在.cc和.hh中添加printf之类的打印信息了。最终的输出结果可以在build/Testing/Temporary/LastTest.log中查看

# 2-Motivation

 在lab0正式开始之前，我想解释一下TCP/IP的实质：

实际上，互联网并没有提供可靠的字节流服务。相反，互联网只做一件事，那就是“尽其最大努力”将数据报文送达目的地

将“尽力而为”的数据报文变成“可靠的字节流”，这其实是通信双方OS的任务，即是互联网的抽象

第一个小测试webget是想让我们体验并模拟一下在浏览器中键入URL后获得远程服务器传来的内容，这并没有太大的难度，因为lab0本身已经为我们搭建好了TCP/IP五层协议栈的框架，我们要做的仅仅是解析传入的host和path即可，具体完善在apps/webget.cc的get_URL()中

第二个基本组件也比较简单，是想让我们实现一个最基本且最常用的字节流，可以将其理解成是一个队列，可以从头部读出数据，也可以从末端写入数据。同样，大体的框架lab0已经实现好了，只需要我们在libsponge/byte_stream.cc中完善其各功能即可

# 3-Solutions

## S1-Writing webget

针对第一个测试点webget，应该在apps/webget.cc中完善具体的`get_URL()`即可，

```cpp
void get_URL(const string &host, const string &path) {
	TCPSocket sock;
  std::string content = "";

  sock.connect(Address(host, "http"));

  content = "GET " + path + " HTTP/1.1\r\nHost: " + host + 
    "\r\nConnection: close\r\n\r\n";
  sock.write(content);

  sock.shutdown(SHUT_WR);

  while (!sock.eof())
    cout << sock.read();

  sock.close();
}
```

首先，进入函数内不管三七二十一，先建socket，然后尝试与目标web主机建立TCP连接。成功后，通过write将获取URL内容的请求发送给服务器。发送完成之后，关闭socket写的Channel，意味着不能再向该socket当中写数据了，或许只能读

之后，就是等待web主机回复。通过eof判断可读的Channel内是否有数据送来，送来一份，读一份，直到读完为止

## S2-An in-memory reliable byte stream

针对第二个测试点稍微复杂一点，要修改libsponge/byte_stream.h和libsponge/byte_stream.cc的内容

ByteStream对象由下面这些成员变量来控制整个stream流程，

```cpp
class ByteStream {
  private:
    std::deque<char> buf_;
    size_t cap_;
    size_t nread_;
    size_t nwrite_;
    bool ended_;
```

首先，`buf_`采用标准库中的双向队列作为缓冲区，存放由TCP层传上来的字节流。`cap_`用来限制缓冲区的空间大小。`nread_`记录上层应用已从缓冲区中读取了多少bytes的数据。而`nwrite_`记录TCP层传上来多少bytes的数据。`ended_`标记这条ByteStream是否应该关闭

首先，要实现`remaining_capacity()`，意在看一下当前对列的容量，从而判断还能否容下将要写入的数据。其定义如下，

```cpp
size_t ByteStream::remaining_capacity() const 
{ 
  return cap_ - buf_.size(); 
}
```

对于writer而言，主要是能够写入数据并且自知状态，包括队列的剩余容量及Channel是否已被关闭。具体由`write()`来实现，

```cpp
size_t ByteStream::write(const string &data) {
  size_t len = data.length();

  /* 有多少容量写多少数据 */
  if(remaining_capacity() < data.length()) {
    len = remaining_capacity();
  }

  for(size_t i=0; i<len; i++) {
    buf_.push_back(data[i]);
  }

  nwrite_ += len;
  return len;
}
```

我们在此采用的是有多少容量，就写多少数据的策略，而不是0或1的策略（非写即丢）。定义了`nwrite_`计数了一次write写了多少数据

`end_input()`较为简单，只分辨是否还有字节将要写入，其定义如下，

```cpp
void ByteStream::end_input() 
{
  ended_ = true; 
}
```

关于读操作，`peek_output()`主要用来查看队首元素，定义如下，

```cpp
string ByteStream::peek_output(const size_t len) const 
{
  if(len > buf_.size()) {
    dbg_printf(DBG_ERR, "[%s][%d]ERROR: len > buf_'s size.\n", __FUNCTION__, __LINE__);    
    return "";
  }

  return string(buf_.begin(), buf_.begin()+len);
}
```

`pop_output()`和查看队首的功能差不多，只是将查看功能改为删除，

```cpp
void ByteStream::pop_output(const size_t len) 
{ 
  if(len > buf_.size()) {
    dbg_printf(DBG_ERR, "[%s][%d]ERROR: len > buf_'s size.\n", __FUNCTION__, __LINE__);    
    return;
  }

  for(size_t i=0; i<len; i++) {
    buf_.pop_front();
  }

  nread_ += len;
}
```

其中的`nread_`和`nwrite_` 样，用来统计reader已经读取了多少数据。`read()`是框架定义好的，如下，

```cpp
std::string read(const size_t len) 
{
  string result = ByteStream::peek_output(len);
  
  pop_output(len);

  return result;
}
```

之后的一系列状态返回函数，我不一一展开了，无非就是封装一下成员变量，对外提供状态查看功能罢了

# 4-Results

每次修改都需要通过`make -j4`重新进行编译。完成编译之后，可以通过make命令来检验自己程序的正确性，

```bash
make check_webget
make check_lab0
```

# 5-Reference

1. [【计算机网络】Stanford CS144 学习笔记](https://blog.csdn.net/kangyupl/article/details/108589594)
2. [[CS144] Lab 0: networking warmup](https://blog.csdn.net/LostUnravel/article/details/124534172)
3. [【斯坦福计网CS144项目】环境配置 & Lab0: ByteStream](https://blog.csdn.net/Altair_alpha/article/details/125010070)