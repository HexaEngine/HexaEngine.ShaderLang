#ifndef STREAM_HPP
#define STREAM_HPP

#include "allocator.h"
#include "config.h"
#include "c/stream.h"
#include "utils/span.hpp"
#include "utils/endianness.hpp"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string>

namespace HXSL
{
	struct Stream
	{
		size_t version;
		void* userdata;
		StreamReadFunc readFunc;
		StreamWriteFunc writeFunc;
		StreamSeekFunc seekFunc;
		StreamGetPositionFunc getPositionFunc;
		StreamGetLengthFunc getLengthFunc;
		StreamFlushFunc flushFunc;
		StreamCloseFunc closeFunc;

		Stream(size_t version, void* userdata, const StreamReadFunc& readFunc, const StreamWriteFunc& writeFunc, const StreamSeekFunc& seekFunc, const StreamGetPositionFunc& getPositionFunc, const StreamGetLengthFunc& getLengthFunc, const StreamFlushFunc& flushFunc, const StreamCloseFunc& closeFunc)
			: version(version), userdata(userdata), readFunc(readFunc), writeFunc(writeFunc), seekFunc(seekFunc), getPositionFunc(getPositionFunc), getLengthFunc(getLengthFunc), flushFunc(flushFunc), closeFunc(closeFunc)
		{
		}

		virtual ~Stream()
		{
			Close();
		}

		size_t Read(void* buffer, size_t size) const
		{
			if (readFunc)
			{
				return readFunc(userdata, buffer, size);
			}
			return EOF;
		}

		size_t Write(const void* buffer, size_t size) const
		{
			if (writeFunc)
			{
				return writeFunc(userdata, buffer, size);
			}
			return EOF;
		}

		int64_t Seek(int64_t offset, SeekOrigin origin) const
		{
			if (seekFunc)
			{
				return seekFunc(userdata, offset, origin);
			}
			return EOF;
		}

		int64_t Position() const
		{
			if (getPositionFunc)
			{
				return getPositionFunc(userdata);
			}
			return EOF;
		}

		void Position(int64_t position) const
		{
			Seek(position, SeekOrigin_Begin);
		}

		int64_t Length() const
		{
			if (getPositionFunc)
			{
				return getLengthFunc(userdata);
			}
			return EOF;
		}

		void Flush() const
		{
			if (flushFunc)
			{
				flushFunc(userdata);
			}
		}

		void Close()
		{
			if (closeFunc)
			{
				Flush();
				closeFunc(userdata);
				userdata = nullptr;
				readFunc = nullptr;
				writeFunc = nullptr;
				seekFunc = nullptr;
				getPositionFunc = nullptr;
				getLengthFunc = nullptr;
				flushFunc = nullptr;
				closeFunc = nullptr;
			}
		}

		template<typename T>
		void WriteValue(const T& value)
		{
			T valueToWrite = EndianUtils::IsLittleEndian() ? value : EndianUtils::ToLittleEndian(value);
			Write(&valueToWrite, sizeof(T));
		}

		template<typename T>
		T ReadValue()
		{
			T value{};
			Read(&value, sizeof(T));
			return EndianUtils::IsLittleEndian() ? value : EndianUtils::FromLittleEndian(value);
		}

		void WriteUInt(uint32_t value)
		{
			WriteValue<uint32_t>(value);
		}

		uint32_t ReadUInt()
		{
			return ReadValue<uint32_t>();
		}

		void WriteString(const std::string& str)
		{
			uint32_t len = static_cast<uint32_t>(str.size());
			WriteUInt(len);
			if (len == 0) return;
			Write(str.data(), len);
		}

		void WriteString(const StringSpan& str)
		{
			uint32_t len = static_cast<uint32_t>(str.length);
			WriteUInt(len);
			if (len == 0) return;
			Write(str.data + str.start, len);
		}

		std::string ReadString()
		{
			uint32_t len = ReadUInt();
			std::string result(len, '\0');
			Read(result.data(), len);
			return result;
		}

		std::string ReadAllText()
		{
			auto len = Length();
			if (len <= 0) return {};

			size_t lenU = static_cast<size_t>(len);
			std::string result;
			result.resize(lenU);
			Read(result.data(), lenU);
			return result;
		}
	};

	struct FileStream : public Stream
	{
		FILE* file;
		FileStream(FILE* file)
			: Stream(sizeof(FileStream), this, FileStreamRead, FileStreamWrite, FileStreamSeek, FileStreamPosition, FileStreamLength, FileStreamFlush, FileStreamClose),
			file(file)
		{
		}

		static std::unique_ptr<FileStream> OpenRead(const char* path)
		{
			FILE* file;
			auto error = fopen_s(&file, path, "rb");
			if (error != 0 || file == nullptr)
			{
				return nullptr;
			}
			return std::make_unique<FileStream>(file);
		}

		static std::unique_ptr<FileStream> OpenCreate(const char* path)
		{
			FILE* file;
			auto error = fopen_s(&file, path, "wb+");
			if (error != 0 || file == nullptr)
			{
				return nullptr;
			}
			return std::make_unique<FileStream>(file);
		}

		static std::unique_ptr<FileStream> Open(const char* path, const char* mode)
		{
			FILE* file;
			auto error = fopen_s(&file, path, mode);
			if (error != 0 || file == nullptr)
			{
				return nullptr;
			}
			return std::make_unique<FileStream>(file);
		}

	private:

		static size_t FileStreamRead(void* userdata, void* buffer, size_t size)
		{
			FileStream* fs = static_cast<FileStream*>(userdata);
			return fread(buffer, 1, size, fs->file);
		}

		static size_t FileStreamWrite(void* userdata, const void* buffer, size_t size)
		{
			FileStream* fs = static_cast<FileStream*>(userdata);
			return fwrite(buffer, 1, size, fs->file);
		}

		static int64_t FileStreamSeek(void* userdata, int64_t offset, SeekOrigin origin)
		{
			FileStream* fs = static_cast<FileStream*>(userdata);
			int originFlag = SEEK_SET;
			switch (origin)
			{
			case SeekOrigin_Begin: originFlag = SEEK_SET; break;
			case SeekOrigin_Current: originFlag = SEEK_CUR; break;
			case SeekOrigin_End: originFlag = SEEK_END; break;
			}
			if (fseek(fs->file, static_cast<long>(offset), originFlag) != 0)
				return -1;
			return ftell(fs->file);
		}

		static int64_t FileStreamPosition(void* userdata)
		{
			FileStream* fs = static_cast<FileStream*>(userdata);
			return ftell(fs->file);
		}

		static int64_t FileStreamLength(void* userdata)
		{
			FileStream* fs = static_cast<FileStream*>(userdata);
			long current = ftell(fs->file);
			fseek(fs->file, 0, SEEK_END);
			long length = ftell(fs->file);
			fseek(fs->file, current, SEEK_SET);
			return length;
		}

		static void FileStreamFlush(void* userdata)
		{
			FileStream* fs = static_cast<FileStream*>(userdata);
			fflush(fs->file);
		}

		static void FileStreamClose(void* userdata)
		{
			FileStream* fs = static_cast<FileStream*>(userdata);
			if (fs->file != nullptr)
			{
				fclose(fs->file);
				fs->file = nullptr;
			}
		}
	};

	struct MemoryStream : public Stream
	{
	private:
		uint8_t* buffer;
		size_t position;
		size_t length;
		size_t capacity;
		bool isDynamic;

		void Reset()
		{
			buffer = nullptr;
			position = 0;
			length = 0;
			capacity = 0;
		}

	public:
		MemoryStream(uint8_t* buffer, size_t size, bool isDynamic) : Stream(sizeof(MemoryStream), this, MemoryStreamRead, MemoryStreamWrite, MemoryStreamSeek, MemoryStreamPosition, MemoryStreamLength, MemoryStreamFlush, MemoryStreamClose),
			buffer(buffer), position(0), length(size), capacity(size), isDynamic(isDynamic)
		{
		}

		MemoryStream(size_t capacity) : Stream(sizeof(MemoryStream), this, MemoryStreamRead, MemoryStreamWrite, MemoryStreamSeek, MemoryStreamPosition, MemoryStreamLength, MemoryStreamFlush, MemoryStreamClose),
			buffer((uint8_t*)Alloc(capacity)), position(0), length(capacity), capacity(capacity), isDynamic(true)
		{
		}

		~MemoryStream()
		{
			if (isDynamic && buffer)
			{
				Free(buffer);
				Reset();
			}
		}

		uint8_t* GetBuffer(bool takeOwnership)
		{
			if (takeOwnership)
			{
				auto temp = buffer;
				Reset();
				return temp;
			}

			return buffer;
		}

		size_t GetBufferSize() const
		{
			return length;
		}

		size_t GetBufferCapacity() const
		{
			return capacity;
		}

		static size_t MemoryStreamRead(void* userdata, void* buffer, size_t size)
		{
			MemoryStream* stream = static_cast<MemoryStream*>(userdata);
			size_t toRead = std::min(size, stream->length - stream->position);
			std::memcpy(buffer, stream->buffer + stream->position, toRead);
			stream->position += toRead;
			return toRead;
		}

		static size_t MemoryStreamWrite(void* userdata, const void* buffer, size_t size)
		{
			auto stream = static_cast<MemoryStream*>(userdata);
			auto capacity = stream->capacity;
			auto newPosition = stream->position + size;

			if (stream->isDynamic && newPosition > capacity)
			{
				auto newCapacity = std::max(capacity * 2, newPosition);
				auto newBuffer = (uint8_t*)ReAlloc(stream->buffer, newCapacity);

				if (newBuffer == nullptr)
				{
					return 0;
				}

				stream->buffer = newBuffer;
				stream->capacity = capacity = newCapacity;
			}
			else if (newPosition > capacity)
			{
				return 0;
			}

			std::memcpy(stream->buffer + stream->position, buffer, size);
			stream->position = newPosition;
			stream->length = std::max(newPosition, stream->length);
			return size;
		}

		static int64_t MemoryStreamSeek(void* userdata, int64_t offset, SeekOrigin origin)
		{
			MemoryStream* stream = static_cast<MemoryStream*>(userdata);
			int64_t newPosition = stream->position;

			switch (origin)
			{
			case SeekOrigin_Begin: newPosition = offset; break;
			case SeekOrigin_Current: newPosition = stream->position + offset; break;
			case SeekOrigin_End: newPosition = stream->length + offset; break;
			}

			if (newPosition < 0 || newPosition > static_cast<int64_t>(stream->length))
			{
				return -1;
			}

			stream->position = newPosition;
			return stream->position;
		}

		static int64_t MemoryStreamPosition(void* userdata)
		{
			MemoryStream* stream = static_cast<MemoryStream*>(userdata);
			return stream->position;
		}

		static int64_t MemoryStreamLength(void* userdata)
		{
			MemoryStream* stream = static_cast<MemoryStream*>(userdata);
			return stream->length;
		}

		static void MemoryStreamFlush(void* userdata)
		{
		}

		static void MemoryStreamClose(void* userdata)
		{
		}
	};
}
#endif