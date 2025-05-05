#include "c/stream.h"
#include "io/stream.hpp"

HXSL_API HXSLStream* HXSL_CreateStream(HXSLStreamDesc* desc)
{
	HXSL::Stream* stream = new HXSL::Stream(desc->version, desc->userdata, desc->readFunc, desc->writeFunc, desc->seekFunc, desc->getPositionFunc, desc->getLengthFunc, desc->flushFunc, desc->closeFunc);
	return reinterpret_cast<HXSLStream*>(stream);
}

HXSL_API HXSLStream* HXSL_CreateFileStream(const char* path)
{
	return reinterpret_cast<HXSLStream*>(HXSL::FileStream::OpenCreate(path).release());
}

HXSL_API HXSLStream* HXSL_ReadFileStream(const char* path)
{
	return reinterpret_cast<HXSLStream*>(HXSL::FileStream::OpenRead(path).release());
}

HXSL_API HXSLStream* HXSL_OpenFileStream(const char* path, const char* mode)
{
	return reinterpret_cast<HXSLStream*>(HXSL::FileStream::Open(path, mode).release());
}

HXSL_API HXSLStream* HXSL_CreateMemoryStream(size_t capacity)
{
	HXSL::Stream* stream = new HXSL::MemoryStream(capacity);
	return reinterpret_cast<HXSLStream*>(stream);
}

HXSL_API HXSLStream* HXSL_CreateMemoryStreamFromBuffer(uint8_t* buffer, size_t size, bool isDynamic)
{
	HXSL::Stream* stream = new HXSL::MemoryStream(buffer, size, isDynamic);
	return reinterpret_cast<HXSLStream*>(stream);
}

HXSL_API uint8_t* HXSL_MemoryStreamGetBuffer(HXSLStream* self, bool takeOwnership)
{
	HXSL::Stream* base = reinterpret_cast<HXSL::Stream*>(self);
	if (base->version < sizeof(HXSL::MemoryStream))
	{
		return nullptr;
	}
	auto* ms = static_cast<HXSL::MemoryStream*>(base);
	return ms->GetBuffer(takeOwnership);
}

HXSL_API int64_t HXSL_MemoryStreamGetBufferSize(HXSLStream* self)
{
	HXSL::Stream* base = reinterpret_cast<HXSL::Stream*>(self);
	if (base->version < sizeof(HXSL::MemoryStream))
	{
		return -1;
	}
	auto* ms = static_cast<HXSL::MemoryStream*>(base);
	return static_cast<int64_t>(ms->GetBufferSize());
}

HXSL_API int64_t HXSL_MemoryStreamGetBufferCapacity(HXSLStream* self)
{
	HXSL::Stream* base = reinterpret_cast<HXSL::Stream*>(self);
	if (base->version < sizeof(HXSL::MemoryStream))
	{
		return -1;
	}
	auto* ms = static_cast<HXSL::MemoryStream*>(base);
	return static_cast<int64_t>(ms->GetBufferCapacity());
}

HXSL_API void HXSL_CloseStream(HXSLStream* self)
{
	if (self == nullptr)
		return;
	HXSL::Stream* base = reinterpret_cast<HXSL::Stream*>(self);
	delete base;
}