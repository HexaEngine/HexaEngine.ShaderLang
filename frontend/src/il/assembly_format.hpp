#pragma once

#include "pch/std.hpp"
#include <io/stream.hpp>
#include <io/version.hpp>
#include <core/module.hpp>

namespace HXSL
{
	enum class LanguageIdentifier : uint32_t
	{
		Unknown = 0,
		HXSL_1_0 = (1 << 24) | (0 << 16) | 0x48585300,
	};

	enum class Architecture : uint16_t
	{
		Unknown,
		Any,
		X86,
		X64,
		ARM64
	};
}

namespace HXSL::AssemblyFormat
{
	using RecordId = Backend::RecordId;

	enum class FormatVersion : uint32_t
	{
		Version1_0 = (1 << 24) | (0 << 16) | (0 << 8) | 0,
	};

	struct AssemblyHeader
	{
		static constexpr char Magic[] = "HXSL";
		static constexpr size_t MagicSize = 4;

		static constexpr FormatVersion MinVersion = FormatVersion::Version1_0;
		static constexpr FormatVersion CurrentVersion = FormatVersion::Version1_0;

		FormatVersion formatVersion = CurrentVersion;
		LanguageIdentifier languageIdentifier = {};
		Architecture architecture = Architecture::Any;
		RecordId entryPoint = {};

		constexpr AssemblyHeader() = default;
		AssemblyHeader(LanguageIdentifier langId, Architecture arch, RecordId entryPoint) : languageIdentifier(langId), architecture(arch), entryPoint(entryPoint)
		{
		}

		void Write(Stream* stream) const
		{
			stream->Write(Magic, MagicSize);
			stream->WriteLittleEndian(formatVersion);
			stream->WriteLittleEndian(languageIdentifier);
			stream->WriteLittleEndian(architecture);
			stream->WriteLittleEndian(entryPoint.value);
		}

		bool Read(Stream* stream)
		{
			char buf[MagicSize];
			stream->Read(buf, MagicSize);
			if (memcmp(buf, Magic, MagicSize) != 0) return false;

			formatVersion = stream->ReadLittleEndian<FormatVersion>();
			if (formatVersion < MinVersion || formatVersion > CurrentVersion)
			{
				return false;
			}

			languageIdentifier = stream->ReadLittleEndian<LanguageIdentifier>();
			architecture = stream->ReadLittleEndian<Architecture>();
			entryPoint = RecordId(stream->ReadLittleEndian<uint64_t>());

			return true;
		}
	};
}