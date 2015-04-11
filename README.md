# Channel
golang channel in c++11

How to use?
#include "Channel.h"

blocking calls

Channel<int> ch;
int i = 0;

ch<<i; // sending i through ch


i<<ch; // receiving i from ch

non-blocking calls

if(i>>ch)
{
  // process
}

if(ch>>i)
{
  // process
}
