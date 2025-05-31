#include "namespace.hpp"
#include "declarations.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "il/assembly_collection.hpp"

namespace HXSL
{
	bool UsingDeclaration::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(Target, AssemblyReferences);
		return AssemblyReferences.size() > 0;
	}

	void Namespace::Write(Stream& stream) const
	{
	}

	void Namespace::Read(Stream& stream, StringPool& container)
	{
	}

	void Namespace::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case SymbolType_Field:
				AddField(UNIQUE_PTR_CAST_AST(nodes[childIdx], Field));
				break;
			case SymbolType_Function:
				AddFunction(UNIQUE_PTR_CAST_AST(nodes[childIdx], FunctionOverload));
				break;
			case SymbolType_Struct:
				AddStruct(UNIQUE_PTR_CAST_AST(nodes[childIdx], Struct));
				break;
			case SymbolType_Class:
				AddClass(UNIQUE_PTR_CAST_AST(nodes[childIdx], Class));
				break;
			}
		}
	}

	void Namespace::Warmup(const AssemblyCollection& references)
	{
		references.FindAssembliesByNamespace(name, this->references);
	}
}