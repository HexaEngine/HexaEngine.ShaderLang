#ifndef STREAM_HPP
#define STREAM_HPP

#include "allocator.h"
#include "config.h"
#include "c/stream.h"
#include "utils/span.hpp"
#include "utils/endianness.hpp"

#include "pch/std.hpp"

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
		void WriteValue(const T& value) const
		{
			T valueToWrite = EndianUtils::IsLittleEndian() ? value : EndianUtils::ToLittleEndian(value);
			Write(&valueToWrite, sizeof(T));
		}

		template<typename T>
		T ReadValue() const
		{
			T value{};
			Read(&value, sizeof(T));
			return EndianUtils::IsLittleEndian() ? value : EndianUtils::FromLittleEndian(value);
		}

		void WriteUInt(uint32_t value) const
		{
			WriteValue<uint32_t>(value);
		}

		uint32_t ReadUInt() const
		{
			return ReadValue<uint32_t>();
		}

		void WriteString(const std::string& str) const
		{
			uint32_t len = static_cast<uint32_t>(str.size());
			WriteUInt(len);
			if (len == 0) return;
			Write(str.data(), len);
		}

		void WriteString(const StringSpan& str) const
		{
			uint32_t len = static_cast<uint32_t>(str.length);
			WriteUInt(len);
			if (len == 0) return;
			Write(str.data + str.start, len);
		}

		std::string ReadString() const
		{
			uint32_t len = ReadUInt();
			std::string result(len, '\0');
			Read(result.data(), len);
			return result;
		}

		std::string ReadAllText() const
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
			buffer((uint8_t*)HXSL_Alloc(capacity)), position(0), length(capacity), capacity(capacity), isDynamic(true)
		{
		}

		~MemoryStream()
		{
			if (isDynamic && buffer)
			{
				HXSL_Free(buffer);
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
				auto newBuffer = (uint8_t*)HXSL_ReAlloc(stream->buffer, newCapacity);

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

	struct BufferedStream : Stream
	{
		enum class LastAction
		{
			None,
			Read,
			Write,
		};

		Stream* inner;
		bool closeInner;
		uint8_t* buffer;
		size_t bufferSize;
		size_t bufferPosition;
		size_t bufferReadSize;
		LastAction lastAction;

		BufferedStream(Stream* inner, bool closeInner = true, size_t bufferSize = 4096)
			: Stream(sizeof(BufferedStream), this, BufferedStreamRead, BufferedStreamWrite, BufferedStreamSeek, BufferedStreamPosition, BufferedStreamLength, BufferedStreamFlush, BufferedStreamClose),
			closeInner(closeInner),
			bufferSize(bufferSize),
			bufferPosition(0),
			bufferReadSize(0),
			lastAction(LastAction::None)
		{
			buffer = new uint8_t[bufferSize];
		}

		BufferedStream(const BufferedStream&) = delete;
		BufferedStream& operator=(const BufferedStream&) = delete;

		static size_t BufferedStreamRead(void* userdata, void* buffer, size_t size)
		{
			if (size == 0) return 0;
			auto stream = static_cast<BufferedStream*>(userdata);
			auto& lastAction = stream->lastAction;
			if (lastAction == LastAction::Write) { stream->Flush(); }

			auto& bufferPosition = stream->bufferPosition;
			auto innerBuffer = stream->buffer;
			const size_t bufferSize = stream->bufferSize;
			auto& bufferReadSize = stream->bufferReadSize;

			auto start = static_cast<uint8_t*>(buffer);
			auto dst = start;

			while (size > 0)
			{
				size_t space = bufferReadSize - bufferPosition;
				if (space == 0)
				{
					size_t read = stream->inner->Read(innerBuffer, bufferSize);
					if (read == EOF)
					{
						return (dst - start);
					}
					bufferPosition = 0;
					bufferReadSize = read;
					space = read;
				}

				size_t toCopy = std::min(space, size);
				std::memcpy(dst, innerBuffer + bufferPosition, toCopy);
				bufferPosition += toCopy;
				dst += toCopy;
				size -= toCopy;
			}

			lastAction = LastAction::Read;
			return (dst - start);
		}

		static size_t BufferedStreamWrite(void* userdata, const void* buffer, size_t size)
		{
			if (size == 0) return 0;
			auto stream = static_cast<BufferedStream*>(userdata);
			auto& lastAction = stream->lastAction;
			if (lastAction == LastAction::Read) { stream->Flush(); }

			auto& bufferPosition = stream->bufferPosition;
			auto innerBuffer = stream->buffer;
			const size_t bufferSize = stream->bufferSize;

			auto start = static_cast<const uint8_t*>(buffer);
			auto src = start;

			while (size > 0)
			{
				size_t space = bufferSize - bufferPosition;
				if (space == 0)
				{
					size_t written = stream->inner->Write(innerBuffer, bufferSize);
					if (written != bufferPosition)
					{
						return written == EOF ? written : (src - start);
					}
					bufferPosition = 0;
					space = bufferSize;
				}

				size_t toCopy = std::min(size, space);
				std::memcpy(innerBuffer + bufferPosition, src, toCopy);
				bufferPosition += toCopy;
				src += toCopy;
				size -= toCopy;
			}

			stream->bufferReadSize = 0;
			lastAction = LastAction::Write;
			return (src - start);
		}

		static int64_t BufferedStreamSeek(void* userdata, int64_t offset, SeekOrigin origin)
		{
			auto stream = static_cast<BufferedStream*>(userdata);
			auto& lastAction = stream->lastAction;
			if (lastAction == LastAction::Write) { stream->Flush(); }
			auto inner = stream->inner;
			auto& bufferReadSize = stream->bufferReadSize;
			auto& bufferPosition = stream->bufferPosition;

			const int64_t oldInnerPositionFront = inner->Position();
			const int64_t oldInnerPosition = oldInnerPositionFront - bufferReadSize;
			const int64_t oldPosition = oldInnerPosition + bufferPosition;
			const size_t length = inner->Length();
			int64_t newPosition = oldPosition;

			switch (origin)
			{
			case SeekOrigin_Begin: newPosition = offset; break;
			case SeekOrigin_Current: newPosition = oldPosition + offset; break;
			case SeekOrigin_End: newPosition = length + offset; break;
			}

			if (newPosition < 0 || newPosition > static_cast<int64_t>(length))
			{
				return EOF;
			}

			if (lastAction == LastAction::Read)
			{
				if (newPosition >= oldInnerPosition && newPosition < oldInnerPositionFront)
				{
					bufferPosition = static_cast<size_t>(newPosition - oldInnerPosition);
					return newPosition;
				}
			}

			bufferReadSize = 0;
			bufferPosition = 0;
			lastAction = LastAction::None;
			return inner->Seek(newPosition, SeekOrigin_Begin);
		}

		static int64_t BufferedStreamPosition(void* userdata)
		{
			auto stream = static_cast<BufferedStream*>(userdata);
			return stream->inner->Position() - stream->bufferReadSize + stream->bufferPosition;
		}

		static int64_t BufferedStreamLength(void* userdata)
		{
			auto stream = static_cast<BufferedStream*>(userdata);
			return stream->inner->Length();
		}

		static void BufferedStreamFlush(void* userdata)
		{
			auto stream = static_cast<BufferedStream*>(userdata);
			auto inner = stream->inner;
			auto& bufferPosition = stream->bufferPosition;
			auto& lastAction = stream->lastAction;

			if (bufferPosition > 0 && lastAction == LastAction::Write)
			{
				inner->Write(stream->buffer, bufferPosition);
				bufferPosition = 0;
			}
			stream->bufferReadSize = 0;
			lastAction = LastAction::None;

			inner->Flush();
		}

		static void BufferedStreamClose(void* userdata)
		{
			auto stream = static_cast<BufferedStream*>(userdata);

			if (stream->buffer == nullptr)
			{
				return;
			}

			delete[] stream->buffer;
			stream->buffer = nullptr;
			stream->bufferSize = 0;
			stream->bufferPosition = 0;
			stream->lastAction = LastAction::None;

			if (stream->closeInner)
			{
				stream->inner->Close();
				stream->inner = nullptr;
				stream->closeInner = false;
			}
		}
	};
}
#endif