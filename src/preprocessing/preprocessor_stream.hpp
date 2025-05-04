#ifndef PREPROCESSOR_STREAM_HPP
#define PREPROCESSOR_STREAM_HPP

#include "utils/span.h"
#include "lexical/lexer_input_stream.hpp"
#include <string>
#include <vector>

namespace HXSL
{
	class PreprocessorStream : public Lexer::InputStream
	{
		std::vector<char> data;
		static constexpr size_t growValue = 1024;
		size_t writePosition;
		size_t readPosition;

		void SetPositionInternal(size_t newPosition) override { readPosition = newPosition; }

		size_t GetPositionInternal() const override { return readPosition; }

		size_t GetLength() const override { return data.size(); }

		size_t ReadInternal(char* buffer, size_t size) { return 0; }

		char ReadThrough() override { if (readPosition >= data.size()) return EOF; return data[readPosition++]; }

	public:

		PreprocessorStream() : Lexer::InputStream(0)
		{
		}

		const char* GetBuffer() override
		{
			return data.data();
		}

		const std::vector<char>& GetData() const { return data; }

		std::vector<char> DetachData() { return std::move(data); }

		void Append(const char* src, size_t length)
		{
			size_t size = data.size();
			size_t nextPosition = writePosition + length;
			if (nextPosition > size)
			{
				size_t newSize = nextPosition + growValue;
				data.resize(newSize);
			}

			std::memcpy(data.data() + writePosition, src, length);
			writePosition = nextPosition;
		}

		void Append(const StringSpan& span)
		{
			Append(span.begin(), span.length);
		}

		void Append(const Token& token)
		{
			Append(token.Span);
		}

		void Insert(size_t index, const char* src, size_t length)
		{
			if (index == writePosition)
			{
				Append(src, length);
				return;
			}

			size_t size = data.size();
			if (index > size)
			{
				throw std::out_of_range("Index is out of bounds");
			}

			size_t nextPosition = writePosition + length;
			if (nextPosition > size)
			{
				size_t newSize = nextPosition + growValue;
				data.resize(newSize);
			}

			std::memmove(data.data() + index + length, data.data() + index, data.size() - index - length);
			std::memcpy(data.data() + index, src, length);
			writePosition = nextPosition;
		}

		void Insert(size_t index, const StringSpan& span)
		{
			Insert(index, span.begin(), span.length);
		}

		void Insert(size_t index, const Token& token)
		{
			Insert(index, token.Span);
		}
	};
}

#endif