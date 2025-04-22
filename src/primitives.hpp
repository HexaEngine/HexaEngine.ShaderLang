#ifndef PRIMITIVES_HPP
#define PRIMITIVES_HPP

#include "config.h"
#include "ast.hpp"
#include "text_span.h"
#include "symbols/symbol_table.hpp"
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
namespace HXSL
{
	static std::string toString(PrimitiveKind kind)
	{
		switch (kind)
		{
		case PrimitiveKind_Void:
			return "void";
			break;
		case PrimitiveKind_Bool:
			return "bool";
			break;
		case PrimitiveKind_Int:
			return "int";
			break;
		case PrimitiveKind_Float:
			return "float";
			break;
		case PrimitiveKind_Uint:
			return "uint";
			break;
		case PrimitiveKind_Double:
			return "double";
			break;
		case PrimitiveKind_Min8Float:
			return "min8float";
			break;
		case PrimitiveKind_Min10Float:
			return "min10float";
			break;
		case PrimitiveKind_Min16Float:
			return "min16float";
			break;
		case PrimitiveKind_Min12Int:
			return "min12int";
			break;
		case PrimitiveKind_Min16Int:
			return "min16int";
			break;
		case PrimitiveKind_Min16Uint:
			return "min16uint";
			break;
		default:
			return "";
			break;
		}
	}

	class PrimitiveManager
	{
	public:
		static PrimitiveManager& GetInstance()
		{
			static PrimitiveManager instance;

			std::call_once(initFlag, []() {
				instance.PrimitiveSymbolTable = std::make_unique<SymbolTable>();
				instance.Populate();
				});

			return instance;
		}

		Primitive* GetPrimitiveType(const TextSpan& name) const;
		const SymbolTable* GetSymbolTable() const
		{
			return PrimitiveSymbolTable.get();
		}

		void Populate();

	private:
		static std::once_flag initFlag;

		PrimitiveManager() = default;

		PrimitiveManager(const PrimitiveManager&) = delete;
		PrimitiveManager& operator=(const PrimitiveManager&) = delete;

		std::unordered_map<TextSpan, std::unique_ptr<Primitive>, TextSpanHash, TextSpanEqual> PrimitiveTypesCache;
		std::unique_ptr<SymbolTable> PrimitiveSymbolTable;

		void AddPrimClass(TextSpan name, Class** outClass = nullptr, size_t* indexOut = nullptr);
		void AddPrim(PrimitiveKind kind, PrimitiveClass primitiveClass, uint rows, uint columns);
		void ResolveInternal(SymbolRef* ref);
	};

	class SwizzleManager
	{
	private:
		std::unique_ptr<SymbolTable> swizzleTable;
		std::vector<std::unique_ptr<SwizzleDefinition>> definitions;
		PrimitiveManager& primitives;

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
		SwizzleManager() : swizzleTable(std::make_unique<SymbolTable>()), primitives(PrimitiveManager::GetInstance())
		{
		}

		bool VerifySwizzle(Primitive* prim, SymbolRef* ref)
		{
			auto _class = prim->GetClass();
			if (_class == PrimitiveClass_Matrix) return false;
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
			auto& resultingType = primitivesTable->GetNode(primitiveIndex).Metadata->declaration;

			auto symbolRef = std::make_unique<SymbolRef>(Token(), SymbolRefType_Member);
			symbolRef->SetTable(primitivesTable, primitiveIndex);
			auto swizzleDef = std::make_unique<SwizzleDefinition>(resultingType->GetSpan(), std::move(symbolRef));
			auto metaField = std::make_shared<SymbolMetadata>(SymbolType_Field, SymbolScopeType_Struct, AccessModifier_Public, 0, swizzleDef.get());

			index = swizzleTable->Insert(pattern, metaField, 0);
			ref->SetTable(swizzleTable.get(), index);

			definitions.push_back(std::move(swizzleDef));

			return true;
		}
	};
}
#endif