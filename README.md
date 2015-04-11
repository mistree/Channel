# Channel
golang channel in c++11

阻塞调用：

    通道<<数据;

    缓冲<<通道;

非阻塞调用：

    数据>>通道;

    通道>>缓冲;


    if(通道>>缓冲)
      非阻塞获取消息成功，处理
    else
      线程做其他事情

How to use?

    #include "Channel.h"

//blocking calls

    Channel<int> ch;
    int i = 0;

    ch<<i; // sending i through ch


    i<<ch; // receiving i from ch

//non-blocking calls

    if(i>>ch)
    {
      // process
    }

    if(ch>>i)
    {
      // process
    }
