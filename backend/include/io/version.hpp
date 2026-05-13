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

		constexpr size_t SizeOf() const { return LEB128Size(major) + LEB128Size(minor) + LEB128Size(patch) + LEB128Size(build); }

		void Write(Stream* stream) const
		{
			stream->WriteLEB128(major);
			stream->WriteLEB128(minor);
			stream->WriteLEB128(patch);
			stream->WriteLEB128(build);
		}

		void Read(Stream* stream)
		{
			major = stream->ReadLEB128<uint32_t>();
			minor = stream->ReadLEB128<uint32_t>();
			patch = stream->ReadLEB128<uint32_t>();
			build = stream->ReadLEB128<uint32_t>();
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