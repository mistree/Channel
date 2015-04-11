#include <queue>
#include <mutex>
#include <condition_variable>

template<class Item>
class Channel {
private:
	std::queue<Item> Buffer;
	std::mutex Locker;
	std::condition_variable Signal;
public:
	friend bool operator << (Item& Out, Channel<Item>& Ch) 
	{
		std::unique_lock<std::mutex> lock(Ch.Locker);
		Ch.Signal.wait(lock, [&](){ return !Ch.Buffer.empty(); });
		if (Ch.Buffer.empty())
			return false;
		Out = Ch.Buffer.front();
		Ch.Buffer.pop();
		return true;
	}

	friend bool operator << (Channel<Item>& Ch, Item In) 
	{
		std::unique_lock<std::mutex> lock(Ch.Locker);
		Ch.Buffer.push(In);
		Ch.Signal.notify_one();	
		return true;
	}

	// non-blocking function
	friend bool operator >> (Channel<Item>& Ch,Item& Out) 
	{
		std::unique_lock<std::mutex> lock(Ch.Locker, defer_lock);
		if (!lock.try_lock())
			return false;		
		if (Ch.Buffer.empty())			
			return false;		
		Out = Ch.Buffer.front();			
		Ch.Buffer.pop();			
		return true;
	}

	friend bool operator >> (Item In, Channel<Item>& Ch) 
	{
		std::unique_lock<std::mutex> lock(Ch.Locker, defer_lock);
		if (!lock.try_lock())
			return false;
		Ch.Buffer.push(In);
		Ch.Signal.notify_one();
		return true;
	}
};
