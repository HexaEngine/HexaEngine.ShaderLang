#pragma once

#include "stream.hpp"

namespace HXSL
{
	struct Version
	{
		uint32_t major = 0;
		uint32_t minor = 0;
		uint32_t patch = 0;
		uint32_t build = 0;

		constexpr Version() : major(0), minor(0), patch(0), build(0) {}
		explicit constexpr Version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0, uint32_t build = 0) : major(major), minor(minor), patch(patch), build(build) {}

		static constexpr size_t SizeOf() { return sizeof(uint32_t) * 4; }

		void Write(Stream* stream) const
		{
			stream->WriteLittleEndian(major);
			stream->WriteLittleEndian(minor);
			stream->WriteLittleEndian(patch);
			stream->WriteLittleEndian(build);
		}

		void Read(Stream* stream)
		{
			major = stream->ReadLittleEndian<uint32_t>();
			minor = stream->ReadLittleEndian<uint32_t>();
			patch = stream->ReadLittleEndian<uint32_t>();
			build = stream->ReadLittleEndian<uint32_t>();
		}

		constexpr bool operator==(const Version& other) const { return major == other.major && minor == other.minor && patch == other.patch && build == other.build; }
		constexpr bool operator!=(const Version& other) const { return !(*this == other); }

		constexpr bool operator>(const Version& other) const { return std::tie(major, minor, patch, build) > std::tie(other.major, other.minor, other.patch, other.build); }
		constexpr bool operator<(const Version& other) const { return std::tie(major, minor, patch, build) < std::tie(other.major, other.minor, other.patch, other.build); }
		constexpr bool operator>=(const Version& other) const { return std::tie(major, minor, patch, build) >= std::tie(other.major, other.minor, other.patch, other.build); }
		constexpr bool operator<=(const Version& other) const { return std::tie(major, minor, patch, build) <= std::tie(other.major, other.minor, other.patch, other.build); }

		size_t hash() const
		{
			XXHash3_64 hash;
			hash.Combine(major);
			hash.Combine(minor);
			hash.Combine(patch);
			hash.Combine(build);
			return static_cast<size_t>(hash.Finalize());
		}
	};
}

namespace std
{
	template<>
	struct hash<HXSL::Version>
	{
		size_t operator()(const HXSL::Version& id) const noexcept
		{
			return id.hash();
		}
	};
}