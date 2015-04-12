#ifndef CHANNEL_H
#define CHANNEL_H

#include <atomic>
#include <mutex>

namespace Dasein
{
	template<typename Item>
	class Channel
	{
	private:
		const int MaxPointer;
		Item* Buffer;
		std::atomic<int> RPointer = 0;
		std::atomic<int> WPointer = 0;
		std::atomic<bool> LastOperation = true; // true read, false write
		std::mutex Reading;
		std::mutex Writing;

	public:
		Channel(int Size): MaxPointer(Size - 1)
		{
			Buffer = new Item[Size + 1];
		};
		~Channel()
		{
			delete[] Buffer;
		}

		bool Full()
		{
			return (RPointer == WPointer && !LastOperation);
		}

		bool Empty()
		{
			return (RPointer == WPointer && LastOperation);
		}

		// non-blocking calls
		friend bool operator <= (Item& Out, Channel<Item>& Ch)
		{
			if (Ch.RPointer == Ch.WPointer && Ch.LastOperation) return false;
			if (!Ch.Reading.try_lock())
				return false;
			Ch.LastOperation = true;
			Out = Ch.Buffer[Ch.RPointer];
			Ch.RPointer = (Ch.RPointer + 1 == Ch.MaxPointer) ? 0 : Ch.RPointer + 1;
			Ch.Reading.unlock();
			return true;
		}

		friend bool operator <= (Channel<Item>& Ch, const Item& In)
		{
			if (Ch.RPointer == Ch.WPointer && !Ch.LastOperation) return false;
			if (!Ch.Writing.try_lock())
				return false;
			Ch.LastOperation = false;
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer + 1 == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			return true;
		}

		friend bool operator <= (Channel<Item>& Ch, const Item&& In)
		{
			if (Ch.RPointer == Ch.WPointer && !Ch.LastOperation) return false;
			if (!Ch.Writing.try_lock())
				return false;
			Ch.LastOperation = false;
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer + 1 == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			return true;
		}

		// blocking calls
		friend bool operator << (Item& Out, Channel<Item>& Ch)
		{
			if (Ch.RPointer == Ch.WPointer && Ch.LastOperation) return false;
			while (!Ch.Reading.try_lock());

			Ch.LastOperation = true;
			Out = Ch.Buffer[Ch.RPointer];
			Ch.RPointer = (Ch.RPointer + 1 == Ch.MaxPointer) ? 0 : Ch.RPointer + 1;
			Ch.Reading.unlock();
			return true;
		}

		friend bool operator << (Channel<Item>& Ch, const Item& In)
		{
			if (Ch.RPointer == Ch.WPointer && !Ch.LastOperation) return false;
			while (!Ch.Writing.try_lock());

			Ch.LastOperation = false;
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer + 1 == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			return true;
		}

		friend bool operator << (Channel<Item>& Ch, const Item&& In)
		{
			if (Ch.RPointer == Ch.WPointer && !Ch.LastOperation) return false;
			while (!Ch.Writing.try_lock());

			Ch.LastOperation = false;
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer + 1 == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			return true;
		}

	};
}

#endif
