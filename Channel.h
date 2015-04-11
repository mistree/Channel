#ifndef CHANNEL_H
#define CHANNEL_H

#include <queue>
#include <mutex>
#include <condition_variable>

namespace Dasein
{
	template<typename Item>
	class Channel {
	private:
		std::queue<Item> Buffer;
		std::mutex Locker;
		std::condition_variable Signal;
		bool mClosed;
	public:
		Channel() : mClosed(false) {};

		void Close()
		{
			std::unique_lock<std::mutex> lock(Locker);
			Buffer.empty();
			mClosed = true;
		}

		bool Closed()
		{
			return mClosed;
		}

		// blockint function
		friend bool operator << (Item& Out, Channel<Item>& Ch)
		{
			std::unique_lock<std::mutex> lock(Ch.Locker);
			Ch.Signal.wait(lock, [&](){ return !Ch.Buffer.empty(); });
			if (Ch.mClosed) throw std::logic_error("Channel already closed");
			if (Ch.Buffer.empty())return false;
			Out = Ch.Buffer.front();
			Ch.Buffer.pop();
			return true;
		}

		// pass by reference
		friend bool operator << (Channel<Item>& Ch, const Item& In)
		{
			std::unique_lock<std::mutex> lock(Ch.Locker);
			if (Ch.mClosed) throw std::logic_error("Channel already closed");
			Ch.Buffer.push(In);
			Ch.Signal.notify_one();
			return true;
		}

		// pass by value
		friend bool operator << (Channel<Item>& Ch, const Item&& In)
		{
			std::unique_lock<std::mutex> lock(Ch.Locker);
			if (Ch.mClosed) throw std::logic_error("Channel already closed");
			Ch.Buffer.push(In);
			Ch.Signal.notify_one();
			return true;
		}

		// non-blocking function
		friend bool operator >> (Channel<Item>& Ch, Item& Out)
		{
			std::unique_lock<std::mutex> lock(Ch.Locker, defer_lock);
			if (!lock.try_lock())return false;
			if (Ch.mClosed) throw std::logic_error("Channel already closed");
			if (Ch.Buffer.empty())
				return false;
			Out = Ch.Buffer.front();
			Ch.Buffer.pop();
			return true;
		}

		// pass by reference
		friend bool operator >> (const Item& In, Channel<Item>& Ch)
		{
			std::unique_lock<std::mutex> lock(Ch.Locker, defer_lock);
			if (!lock.try_lock())return false;
			if (Ch.mClosed) throw std::logic_error("Channel already closed");
			Ch.Buffer.push(In);
			Ch.Signal.notify_one();
			return true;
		}

		// pass by value
		friend bool operator >> (const Item&& In, Channel<Item>& Ch)
		{
			std::unique_lock<std::mutex> lock(Ch.Locker, defer_lock);
			if (!lock.try_lock())return false;
			if (Ch.mClosed) throw std::logic_error("Channel already closed");
			Ch.Buffer.push(In);
			Ch.Signal.notify_one();
			return true;
		}
	};
}

#endif
