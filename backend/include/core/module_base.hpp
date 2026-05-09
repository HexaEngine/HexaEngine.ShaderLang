#pragma once

#include "pch/std.hpp"
#include "language.hpp"

#include "utils/span.hpp"
#include "utils/static_vector.hpp"
#include "utils/bump_allocator.hpp"
#include "utils/dense_set.hpp"

#include "utils/rtti_helper.hpp"
#include "io/stream.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILContext;
		class ILCodeBlob;
		class Module;
		class Layout;
		class TypeLayout;
		class FunctionLayout;
		class OperatorLayout;
		class ConstructorLayout;
		class FieldLayout;
		class StructLayout;
		class EnumLayout;
		class EnumItemLayout;
		class PrimitiveLayout;
		class PointerLayout;
		class NamespaceLayout;

		enum class LayoutFlags : uint8_t
		{
			None = 0,
			Extern = 1,
		};

		DEFINE_FLAGS_OPERATORS(LayoutFlags, char);

		enum class LayoutType : uint8_t
		{
			Unknown,
			ModuleLayoutType,
			NamespaceLayoutType,
			ParameterLayoutType,
			FunctionLayoutType,
			OperatorLayoutType,
			ConstructorLayoutType,
			FieldLayoutType,
			StructLayoutType,
			EnumLayoutType,
			EnumItemLayoutType,
			PrimitiveLayoutType,
			PointerLayoutType,
		};

		struct RecordId
		{
			static constexpr uint64_t ImportFlag = 1ULL << 63;
			static constexpr uint64_t NullId = 0;

			explicit constexpr RecordId(uint64_t value) : value(value) {}
			constexpr RecordId() : value(0) {}

			uint64_t value = 0;

			constexpr bool IsNull() const noexcept { return value == NullId; }
			constexpr bool IsImport() const noexcept { return (value & ImportFlag) != 0; }
			constexpr bool IsExport() const noexcept { return value != 0 && (value & ImportFlag) == 0; }
			constexpr uint64_t Index() const noexcept { return value & ~ImportFlag; }

			static constexpr RecordId Export(uint64_t index) noexcept { return RecordId(index); }
			static constexpr RecordId Import(uint64_t index) noexcept { return RecordId(index | ImportFlag); }

			constexpr bool operator==(RecordId other) const noexcept { return value == other.value; }
			constexpr bool operator!=(RecordId other) const noexcept { return !(*this == other); }
		};

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

		namespace LayoutDataTypes
		{
			using ModuleId = uint32_t;
			using ModuleIdCount = ModuleId;
			using RecordId = ::HXSL::Backend::RecordId;
			using RecordSize = uint32_t;
		}
	}
}

namespace std
{
	template<>
	struct hash<HXSL::Backend::RecordId>
	{
		size_t operator()(HXSL::Backend::RecordId id) const noexcept
		{
			return std::hash<uint64_t>{}(id.value);
		}
	};

	template<>
	struct hash<HXSL::Backend::Version>
	{
		size_t operator()(const HXSL::Backend::Version& id) const noexcept
		{
			return id.hash();
		}
	};
}