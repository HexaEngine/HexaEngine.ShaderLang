#ifndef LEXER_INPUT_STREAM_HPP
#define LEXER_INPUT_STREAM_HPP

#include "c/stream.h"
#include "io/stream.hpp"

#include <vector>

namespace HXSL
{
	namespace Lexer
	{
		class InputStream
		{
		private:
			std::vector<char> buffer;
			size_t bufferPosition;
			size_t bufferSize;
			size_t bufferCapacity;
		protected:
			InputStream(size_t bufferCapacity = 1024) : bufferSize(0), bufferCapacity(bufferCapacity)
			{
				buffer.resize(bufferCapacity);
			}

			virtual void SetPositionInternal(size_t newPosition) = 0;
			virtual size_t GetPositionInternal() const = 0;
			virtual size_t ReadInternal(char* dst, size_t size) = 0;
			virtual char ReadThrough() = 0;

			size_t FillBuffer()
			{
				bufferSize = ReadInternal(buffer.data(), bufferCapacity);
				return bufferSize;
			}

		public:

			virtual size_t GetLength() const = 0;
			size_t GetPosition() const
			{
				auto innerPos = GetPositionInternal();
				return innerPos == 0 ? bufferPosition : innerPos - bufferSize + bufferPosition;
			}

			virtual const char* GetBuffer()
			{
				return buffer.data();
			}

			char ReadChar()
			{
				if (bufferCapacity == 0)
				{
					return ReadThrough();
				}
				else
				{
					if (bufferPosition >= bufferSize)
					{
						auto result = FillBuffer();
						if (result == 0 || result < bufferPosition)
						{
							return EOF;
						}
					}

					return buffer[bufferPosition++];
				}
			}

			bool IsEOF() const
			{
				return GetPosition() >= GetLength();
			}

			void SetPosition(size_t newPosition)
			{
				if (bufferCapacity == 0)
				{
					SetPositionInternal(newPosition);
					return;
				}

				size_t bufferEnd = GetPositionInternal();
				if (bufferEnd == 0)
				{
					size_t alignedPosition = (newPosition / bufferCapacity) * bufferCapacity;
					bufferPosition = newPosition - alignedPosition;
					SetPositionInternal(alignedPosition);
					return;
				}

				size_t currentPosition = bufferEnd - bufferSize;
				if (newPosition >= currentPosition && newPosition < bufferEnd)
				{
					bufferPosition = newPosition - currentPosition;
					return;
				}

				bufferSize = 0;
				size_t alignedPosition = (newPosition / bufferCapacity) * bufferCapacity;
				bufferPosition = newPosition - alignedPosition;
				SetPositionInternal(alignedPosition);
			}

			void Reset()
			{
				SetPosition(0);
				bufferPosition = 0;
				bufferSize = 0;
			}
		};
	}
}

#endif