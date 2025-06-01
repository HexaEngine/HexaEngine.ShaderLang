#ifndef TEXT_STREAM_HPP
#define TEXT_STREAM_HPP

#include "stream.hpp"
#include "utils/span.hpp"

namespace HXSL
{
	class TextStream
	{
		std::vector<char> data;
		static constexpr size_t growValue = 1024;
		size_t writePosition;
		size_t dataSize;

	public:
		TextStream() : writePosition(0), dataSize(0)
		{
		}

		const char* GetBuffer() const noexcept
		{
			return data.data();
		}

		void SetLength(size_t size)  noexcept
		{
			dataSize = size;
			writePosition = std::min(writePosition, size);
		}

		size_t GetLength() const noexcept
		{
			return dataSize;
		}

		size_t GetPosition() const noexcept { return writePosition; }

		std::vector<char>& GetData() { return data; }

		std::vector<char> DetachData() { return std::move(data); }

		void Reserve(size_t wantedCapacity)
		{
			if (data.size() < wantedCapacity)
			{
				data.resize(wantedCapacity);
			}
		}

		bool CopyFrom(Stream& stream)
		{
			auto len = stream.Length();
			if (len < 0) return false;
			if (len == 0) return true;
			size_t length = static_cast<size_t>(len);

			size_t capacity = data.size();
			size_t nextPosition = writePosition + length;
			if (nextPosition > capacity)
			{
				size_t newSize = nextPosition + growValue;
				data.resize(newSize);
			}

			auto read = stream.Read(data.data() + writePosition, length);
			if (read != length)
			{
				return false;
			}
			writePosition = nextPosition;
			dataSize = std::max(dataSize, nextPosition);
			return true;
		}

		void CopyTo(TextStream* stream)
		{
			stream->Write(data.data(), dataSize);
		}

		void Write(const char* src, size_t length)
		{
			if (length == 0) return;
			size_t capacity = data.size();
			size_t nextPosition = writePosition + length;
			if (nextPosition > capacity)
			{
				size_t newSize = nextPosition + growValue;
				data.resize(newSize);
			}

			std::memcpy(data.data() + writePosition, src, length);
			writePosition = nextPosition;
			dataSize = std::max(dataSize, nextPosition);
		}

		void Write(const StringSpan& span)
		{
			Write(span.begin(), span.length);
		}

		void Print()
		{
			std::cout << std::string(data.data(), dataSize) << std::endl;
		}
	};
}

#endif