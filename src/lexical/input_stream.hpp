#ifndef LEXER_INPUT_STREAM_HPP
#define LEXER_INPUT_STREAM_HPP

#include "io/stream.hpp"
#include "utils/span.hpp"

#include <vector>

namespace HXSL
{
	namespace Lexer
	{
		using HXSL::StringSpan;

		class InputStream
		{
			std::vector<char> data;
			static constexpr size_t growValue = 1024;
			size_t writePosition;
			size_t dataSize;

		public:
			InputStream() : writePosition(0), dataSize(0)
			{
			}

			const char* GetBuffer() const noexcept
			{
				return data.data();
			}

			size_t GetLength() const noexcept
			{
				return dataSize;
			}

			const std::vector<char>& GetData() const { return data; }

			std::vector<char> DetachData() { return std::move(data); }

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

			void Insert(size_t index, const char* src, size_t length)
			{
				if (length == 0) return;

				if (index == writePosition)
				{
					Write(src, length);
					return;
				}

				size_t capacity = data.size();
				if (index > dataSize)
				{
					throw std::out_of_range("Index is out of bounds");
				}

				size_t nextPosition = writePosition + length;
				if (nextPosition > capacity)
				{
					size_t newSize = nextPosition + growValue;
					data.resize(newSize);
				}

				std::memmove(data.data() + index + length, data.data() + index, dataSize - index - length);
				std::memcpy(data.data() + index, src, length);

				if (index < writePosition)
				{
					writePosition = nextPosition;
				}

				dataSize += length;
			}

			void Insert(size_t index, const StringSpan& span)
			{
				Insert(index, span.begin(), span.length);
			}
		};
	}
}

#endif