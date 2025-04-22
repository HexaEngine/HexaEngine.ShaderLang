#ifndef STREAM_H
#define STREAM_H

#include "text_span.h"
#include "config.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string>

namespace EndianUtils
{
	inline bool IsLittleEndian()
	{
		const int value{ 0x01 };
		const void* address{ static_cast<const void*>(&value) };
		const unsigned char* least_significant_address{ static_cast<const unsigned char*>(address) };

		return (*least_significant_address == 0x01);
	}

	template <typename T>
	T ToLittleEndian(T value)
	{
		static_assert(std::is_integral<T>::value, "Integral type required.");
		if (!IsLittleEndian()) {
			if constexpr (sizeof(T) == 1) {
				return value;
			}
			else if constexpr (sizeof(T) == 2) {
				return static_cast<T>((static_cast<uint16_t>(value) >> 8) | (static_cast<uint16_t>(value) << 8));
			}
			else if constexpr (sizeof(T) == 4) {
				return static_cast<T>((static_cast<uint32_t>(value) >> 24) |
					((static_cast<uint32_t>(value) & 0x00FF0000) >> 8) |
					((static_cast<uint32_t>(value) & 0x0000FF00) << 8) |
					((static_cast<uint32_t>(value) & 0x000000FF) << 24));
			}
			else if constexpr (sizeof(T) == 8) {
				return static_cast<T>((static_cast<uint64_t>(value) >> 56) |
					((static_cast<uint64_t>(value) & 0x00FF000000000000) >> 40) |
					((static_cast<uint64_t>(value) & 0x0000FF0000000000) >> 24) |
					((static_cast<uint64_t>(value) & 0x000000FF00000000) >> 8) |
					((static_cast<uint64_t>(value) & 0x00000000FF000000) << 8) |
					((static_cast<uint64_t>(value) & 0x0000000000FF0000) << 24) |
					((static_cast<uint64_t>(value) & 0x000000000000FF00) << 40) |
					((static_cast<uint64_t>(value) & 0x00000000000000FF) << 56));
			}
		}
		return value;
	}

	template <typename T>
	T FromLittleEndian(T value)
	{
		static_assert(std::is_integral<T>::value, "Integral type required.");
		return ToLittleEndian(value);
	}
}
namespace HXSL
{
	enum SeekOrigin
	{
		SeekOrigin_Begin = 0,
		SeekOrigin_Current = 1,
		SeekOrigin_End = 2
	};

	typedef size_t(*StreamReadFunc)(void* userdata, void* buffer, size_t size);
	typedef size_t(*StreamWriteFunc)(void* userdata, const void* buffer, size_t size);
	typedef int64_t(*StreamSeekFunc)(void* userdata, int64_t offset, SeekOrigin origin);
	typedef int64_t(*StreamGetPositionFunc)(void* userdata);
	typedef int64_t(*StreamGetLengthFunc)(void* userdata);

	struct Stream
	{
	private:
		void* userdata;
		StreamReadFunc readFunc;
		StreamWriteFunc writeFunc;
		StreamSeekFunc seekFunc;
		StreamGetPositionFunc getPositionFunc;
		StreamGetLengthFunc getLengthFunc;
	public:
		Stream(void* userdata, const StreamReadFunc& readFunc, const StreamWriteFunc& writeFunc, const StreamSeekFunc& seekFunc, const StreamGetPositionFunc& getPositionFunc, const StreamGetLengthFunc& getLengthFunc)
			: userdata(userdata), readFunc(readFunc), writeFunc(writeFunc), seekFunc(seekFunc), getPositionFunc(getPositionFunc), getLengthFunc(getLengthFunc)
		{
		}

		void Read(void* buffer, size_t size)
		{
			if (readFunc)
			{
				readFunc(userdata, buffer, size);
			}
		}

		void Write(const void* buffer, size_t size)
		{
			if (writeFunc)
			{
				writeFunc(userdata, buffer, size);
			}
		}

		int64_t Seek(int64_t offset, SeekOrigin origin)
		{
			if (seekFunc)
			{
				return seekFunc(userdata, offset, origin);
			}
			return -1;
		}

		int64_t Position()
		{
			if (getPositionFunc)
			{
				return getPositionFunc(userdata);
			}
			return -1;
		}

		void Position(int64_t position)
		{
			Seek(position, SeekOrigin_Begin);
		}

		int64_t Length()
		{
			if (getPositionFunc)
			{
				return getLengthFunc(userdata);
			}
			return -1;
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

		void WriteUInt(uint value)
		{
			WriteValue<uint>(value);
		}

		uint ReadUInt()
		{
			return ReadValue<uint>();
		}

		void WriteString(const std::string& str)
		{
			uint len = static_cast<uint>(str.size());
			WriteUInt(len);
			if (len == 0) return;
			Write(str.data(), len);
		}

		void WriteString(const TextSpan& str)
		{
			uint len = static_cast<uint>(str.Length);
			WriteUInt(len);
			if (len == 0) return;
			Write(str.Text + str.Start, len);
		}

		std::string ReadString()
		{
			uint len = ReadUInt();
			std::string result(len, '\0');
			Read(result.data(), len);
			return result;
		}
	};

	struct FileStream : public Stream
	{
		FileStream(FILE* file)
			: Stream(file, FileStreamRead, FileStreamWrite, FileStreamSeek, FileStreamPosition, FileStreamLength)
		{
		}

		static size_t FileStreamRead(void* userdata, void* buffer, size_t size)
		{
			FILE* file = static_cast<FILE*>(userdata);
			return fread(buffer, 1, size, file);
		}

		static size_t FileStreamWrite(void* userdata, const void* buffer, size_t size)
		{
			FILE* file = static_cast<FILE*>(userdata);
			return fwrite(buffer, 1, size, file);
		}

		static int64_t FileStreamSeek(void* userdata, int64_t offset, SeekOrigin origin)
		{
			FILE* file = static_cast<FILE*>(userdata);
			int originFlag = SEEK_SET;
			switch (origin)
			{
			case SeekOrigin_Begin: originFlag = SEEK_SET; break;
			case SeekOrigin_Current: originFlag = SEEK_CUR; break;
			case SeekOrigin_End: originFlag = SEEK_END; break;
			}
			if (fseek(file, static_cast<long>(offset), originFlag) != 0)
				return -1;
			return ftell(file);
		}

		static int64_t FileStreamPosition(void* userdata)
		{
			FILE* file = static_cast<FILE*>(userdata);
			return ftell(file);
		}

		static int64_t FileStreamLength(void* userdata)
		{
			FILE* file = static_cast<FILE*>(userdata);
			long current = ftell(file);
			fseek(file, 0, SEEK_END);
			long length = ftell(file);
			fseek(file, current, SEEK_SET);
			return length;
		}
	};
}
#endif