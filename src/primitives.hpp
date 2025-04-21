#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "config.h"
#include "nodes.hpp"
#include "text_span.h"
#include "symbol_table.hpp"
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
namespace HXSL
{
	static std::string toString(HXSLPrimitiveKind kind)
	{
		switch (kind)
		{
		case HXSLPrimitiveKind_Void:
			return "void";
			break;
		case HXSLPrimitiveKind_Bool:
			return "bool";
			break;
		case HXSLPrimitiveKind_Int:
			return "int";
			break;
		case HXSLPrimitiveKind_Float:
			return "float";
			break;
		case HXSLPrimitiveKind_Uint:
			return "uint";
			break;
		case HXSLPrimitiveKind_Double:
			return "double";
			break;
		case HXSLPrimitiveKind_Min8Float:
			return "min8float";
			break;
		case HXSLPrimitiveKind_Min10Float:
			return "min10float";
			break;
		case HXSLPrimitiveKind_Min16Float:
			return "min16float";
			break;
		case HXSLPrimitiveKind_Min12Int:
			return "min12int";
			break;
		case HXSLPrimitiveKind_Min16Int:
			return "min16int";
			break;
		case HXSLPrimitiveKind_Min16Uint:
			return "min16uint";
			break;
		default:
			return "";
			break;
		}
	}

	class HXSLPrimitiveManager
	{
	public:
		static HXSLPrimitiveManager& GetInstance()
		{
			static HXSLPrimitiveManager instance;

			std::call_once(initFlag, []() {
				instance.PrimitiveSymbolTable = std::make_unique<SymbolTable>();
				instance.Populate();
				});

			return instance;
		}

		HXSLPrimitive* GetPrimitiveType(const TextSpan& name) const;
		const SymbolTable* GetSymbolTable() const
		{
			return PrimitiveSymbolTable.get();
		}

		void Populate();

	private:
		static std::once_flag initFlag;

		HXSLPrimitiveManager() = default;

		HXSLPrimitiveManager(const HXSLPrimitiveManager&) = delete;
		HXSLPrimitiveManager& operator=(const HXSLPrimitiveManager&) = delete;

		std::unordered_map<TextSpan, std::unique_ptr<HXSLPrimitive>, TextSpanHash, TextSpanEqual> PrimitiveTypesCache;
		std::unique_ptr<SymbolTable> PrimitiveSymbolTable;

		void AddPrimClass(TextSpan name, HXSLClass** outClass = nullptr, size_t* indexOut = nullptr);
		void AddPrim(HXSLPrimitiveKind kind, HXSLPrimitiveClass primitiveClass, uint rows, uint columns);
		void ResolveInternal(HXSLSymbolRef* ref);
	};

	class HXSLSwizzleManager
	{
	private:
		std::unique_ptr<SymbolTable> swizzleTable;
		std::vector<std::unique_ptr<HXSLSwizzleDefinition>> definitions;
		HXSLPrimitiveManager& primitives;

		static char NormalizeSwizzleChar(const char& c)
		{
			switch (c) {
			case 'r': return 'x';
			case 'g': return 'y';
			case 'b': return 'z';
			case 'a': return 'w';
			case 's': return 'x';
			case 't': return 'y';
			case 'p': return 'z';
			case 'q': return 'w';
			default: return c;
			}
		}
		static int SwizzleCharToIndex(char c)
		{
			switch (c) {
			case 'x': return 0;
			case 'y': return 1;
			case 'z': return 2;
			case 'w': return 3;
			default: return -1;
			}
		}

	public:
		HXSLSwizzleManager() : swizzleTable(std::make_unique<SymbolTable>()), primitives(HXSLPrimitiveManager::GetInstance())
		{
		}

		bool VerifySwizzle(HXSLPrimitive* prim, HXSLSymbolRef* ref)
		{
			auto _class = prim->GetClass();
			if (_class == HXSLPrimitiveClass_Matrix) return false;
			auto& pattern = ref->GetSpan();
			if (pattern.Length < 1 || pattern.Length > 4)
				return false;

			auto index = swizzleTable->FindNodeIndexPart(pattern);

			if (index != -1)
			{
				ref->SetTable(swizzleTable.get(), index);
				return true;
			}

			auto componentCount = (int)prim->GetRows();

			for (auto& c : pattern)
			{
				char n = NormalizeSwizzleChar(c);
				int i = SwizzleCharToIndex(n);
				if (i < 0 || i >= componentCount)
				{
					return false;
				}
			}

			std::string typeName = toString(prim->GetKind()) + std::to_string(pattern.Length);

			auto primitivesTable = primitives.GetSymbolTable();
			auto primitiveIndex = primitivesTable->FindNodeIndexPart(typeName);
			auto& resultingType = primitivesTable->GetNode(primitiveIndex).Metadata->Declaration;

			auto symbolRef = std::make_unique<HXSLSymbolRef>(Token(), HXSLSymbolRefType_Member);
			symbolRef->SetTable(primitivesTable, primitiveIndex);
			auto swizzleDef = std::make_unique<HXSLSwizzleDefinition>(resultingType->GetSpan(), std::move(symbolRef));
			auto metaField = std::make_shared<SymbolMetadata>(HXSLSymbolType_Field, HXSLSymbolScopeType_Struct, HXSLAccessModifier_Public, 0, swizzleDef.get());

			index = swizzleTable->Insert(pattern, metaField, 0);
			ref->SetTable(swizzleTable.get(), index);

			definitions.push_back(std::move(swizzleDef));

			return true;
		}
	};
}
#endif