#pragma once

#include <utils/bump_allocator.hpp>
#include <utils/dense_map.hpp>
#include <utils/span.hpp>
#include <utils/string_pool.hpp>

namespace HXSL
{
	namespace Backend
	{
		enum class OptionType
		{
			Unknown,
			Bool,
			U8,
			U16,
			U32,
			U64,
			I8,
			I16,
			I32,
			I64,
			F32,
			F64,
			Pointer,
			String
		};

		struct OptionValue
		{
			OptionType type;
			union
			{
				bool b;
				uint8_t u8;
				uint16_t u16;
				uint32_t u32;
				uint64_t u64;
				int8_t i8;
				int16_t i16;
				int32_t i32;
				int64_t i64;
				float f32;
				double f64;
				void* ptr;
				const char* string;
			};
		};

		template<OptionType type>
		struct OptionTypeT
		{
			using T = void;
		};

#define DEFINE_OPTION_TYPE_T(optionType, type, field) \
		template<> \
		struct OptionTypeT<optionType> \
		{ \
			using T = type; \
			static void WriteValue(OptionValue& dst, T value) { dst.field = value; } \
			static T ReadValue(const OptionValue& src) { return src.field; } \
		};

		DEFINE_OPTION_TYPE_T(OptionType::Bool, bool, b);
		DEFINE_OPTION_TYPE_T(OptionType::U8, uint8_t, u8);
		DEFINE_OPTION_TYPE_T(OptionType::U16, uint16_t, u16);
		DEFINE_OPTION_TYPE_T(OptionType::U32, uint32_t, u32);
		DEFINE_OPTION_TYPE_T(OptionType::U64, uint64_t, u64);
		DEFINE_OPTION_TYPE_T(OptionType::I8, int8_t, i8);
		DEFINE_OPTION_TYPE_T(OptionType::I16, int16_t, i16);
		DEFINE_OPTION_TYPE_T(OptionType::I32, int32_t, i32);
		DEFINE_OPTION_TYPE_T(OptionType::I64, int64_t, i64);
		DEFINE_OPTION_TYPE_T(OptionType::F32, float, f32);
		DEFINE_OPTION_TYPE_T(OptionType::F64, double, f64);
		DEFINE_OPTION_TYPE_T(OptionType::Pointer, void*, ptr);
		DEFINE_OPTION_TYPE_T(OptionType::String, const char*, string);

		template<OptionType type>
		using OptionTypeT_t = OptionTypeT<type>::T;

		template<OptionType type, const char* t>
		struct OptionDescriptor
		{
			static constexpr size_t cstrlen(const char* c)
			{
				size_t s = 0;
				while (*c++ != '\0') ++s;
				return s;
			}

			static constexpr OptionType Type = type;
			static constexpr StringSpan Key = { t, cstrlen(t) };
		};

		class OptionCollection
		{
			dense_map<StringSpan, OptionValue> map;
			StringPool2 pool;

		public:
			template<typename TDesc>
			void Set(OptionTypeT_t<TDesc::Type> value)
			{
				using TypeFacade = OptionTypeT<TDesc::Type>;
				using T = TypeFacade::T;
				constexpr StringSpan key = TDesc::Key;

				auto keyPooled = pool.add(key);

				auto it = map.insert({ keyPooled , {} });
				TypeFacade::WriteValue(it.first->second, value);
			}

			template<typename TDesc>
			OptionTypeT_t<TDesc::Type> Get(OptionTypeT_t<TDesc::Type> defaultValue = {}) const
			{
				using TypeFacade = OptionTypeT<TDesc::Type>;
				using T = TypeFacade::T;
				constexpr StringSpan key = TDesc::Key;

				auto it = map.find(key);
				if (it == map.end())
				{
					return defaultValue;
				}

				return TypeFacade::ReadValue(it->second);
			}
		};

		static constexpr char InlinerAggressiveInlineKey[] = "Inliner.AggressiveInline";
		using InlinerAggressiveInline = OptionDescriptor<OptionType::Bool, InlinerAggressiveInlineKey>;

		static constexpr char InlinerInlineExternKey[] = "Inliner.InlineExtern";
		using InlinerInlineExtern = OptionDescriptor<OptionType::Bool, InlinerInlineExternKey>;
	}
}