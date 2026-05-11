#pragma once

#include "pch/std.hpp"
#include "language.hpp"

#include "utils/span.hpp"
#include "utils/static_vector.hpp"
#include "utils/bump_allocator.hpp"
#include "utils/dense_set.hpp"

#include "utils/rtti_helper.hpp"
#include "io/stream.hpp"
#include "io/version.hpp"

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

		class ModuleReader;
		class ModuleWriter;

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

		namespace LayoutDataTypes
		{
			using ModuleId = uint32_t;
			using ModuleIdCount = ModuleId;
			using RecordId = ::HXSL::Backend::RecordId;
			using RecordSize = uint32_t;

			enum class ModuleFormatVersion : uint32_t
			{
				Unknown,
				Version1_0 = (1 << 24) | (0 << 16) | (0 << 8) | 0,
			};

			struct ModuleHeader
			{
				static constexpr ModuleFormatVersion MinVersion = ModuleFormatVersion::Version1_0;
				static constexpr ModuleFormatVersion CurrentVersion = ModuleFormatVersion::Version1_0;
				ModuleFormatVersion version = CurrentVersion;

				uint64_t moduleReferenceTableSize = 0;
				uint64_t exportTableSize = 0;
				uint64_t importTableSize = 0;
				uint64_t recordSectionSize = 0;

				void Write(Stream* stream) const
				{
					stream->WriteLittleEndian(version);
					stream->WriteLittleEndian(moduleReferenceTableSize);
					stream->WriteLittleEndian(exportTableSize);
					stream->WriteLittleEndian(importTableSize);
					stream->WriteLittleEndian(recordSectionSize);
				}

				bool Read(Stream* stream)
				{
					version = stream->ReadLittleEndian<ModuleFormatVersion>();
					if (version < MinVersion || version > CurrentVersion) return false;

					moduleReferenceTableSize = stream->ReadLittleEndian<uint64_t>();
					exportTableSize = stream->ReadLittleEndian<uint64_t>();
					importTableSize = stream->ReadLittleEndian<uint64_t>();
					recordSectionSize = stream->ReadLittleEndian<uint64_t>();

					return true;
				}

				static constexpr size_t SizeOf() { return sizeof(ModuleFormatVersion) + sizeof(uint64_t) * 4; }
			};
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
}