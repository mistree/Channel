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
		const unsigned int MaxPointer;
		const unsigned int Size;
		Item* Buffer;
		unsigned int RPointer = 0;
		unsigned int WPointer = 0;
		std::mutex Reading;
		std::mutex Writing;
		std::atomic<unsigned int> Elements = 0;
		

	public:
		Channel(unsigned int Size) 
			: MaxPointer(Size - 1), Buffer(new Item[Size]), Size(Size)
		{};
		~Channel()
		{
			delete[] Buffer;
		}

		bool Full()
		{
			return Elements == Ch.Size;
		}

		bool Empty()
		{
			return Elements == 0;
		}

		// non-blocking calls
		friend bool operator <= (Item& Out, Channel<Item>& Ch)
		{
			// return if empty
			if (Ch.Elements == 0) return false;
			if (!Ch.Reading.try_lock())
				return false;
			Out = Ch.Buffer[Ch.RPointer];
			Ch.RPointer = (Ch.RPointer == Ch.MaxPointer) ? 0 : Ch.RPointer + 1;		
			Ch.Reading.unlock();
			Ch.Elements--;
			return true;
		}

		friend bool operator <= (Channel<Item>& Ch, const Item& In)
		{
			// return if full
			if (Ch.Elements == Ch.Size) return false;
			if (!Ch.Writing.try_lock())
				return false;
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			Ch.Elements++;
			return true;
		}

		friend bool operator <= (Channel<Item>& Ch, const Item&& In)
		{
			// return if full
			if (Ch.Elements == Ch.Size) return false;
			if (!Ch.Writing.try_lock())
				return false;
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			Ch.Elements++;
			return true;
		}

		// blocking calls
		friend bool operator << (Item& Out, Channel<Item>& Ch)
		{
			// return if empty
			if (Ch.Elements == 0) return false;
			while (!Ch.Reading.try_lock());
			Out = Ch.Buffer[Ch.RPointer];
			Ch.RPointer = (Ch.RPointer == Ch.MaxPointer) ? 0 : Ch.RPointer + 1;
			Ch.Reading.unlock();
			Ch.Elements--;
			return true;
		}

		friend bool operator << (Channel<Item>& Ch, const Item& In)
		{
			// return if full
			if (Ch.Elements == Ch.Size) return false;
			while (!Ch.Writing.try_lock());
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			Ch.Elements++;
			return true;
		}

		friend bool operator << (Channel<Item>& Ch, const Item&& In)
		{
			// return if full
			if (Ch.Elements == Ch.Size) return false;
			while (!Ch.Writing.try_lock());
			Ch.Buffer[Ch.WPointer] = In;
			Ch.WPointer = (Ch.WPointer == Ch.MaxPointer) ? 0 : Ch.WPointer + 1;
			Ch.Writing.unlock();
			Ch.Elements++;
			return true;
		}

	};
}

#endif
