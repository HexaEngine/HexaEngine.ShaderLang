#include "declarations.hpp"
#include "semantics/symbols/symbol_table.hpp"

namespace HXSL
{
	void Struct::Write(Stream& stream) const
	{
	}

	void Struct::Read(Stream& stream, StringPool& container)
	{
	}

	void Struct::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
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
			case SymbolType_Operator:
				AddOperator(UNIQUE_PTR_CAST_AST(nodes[childIdx], OperatorOverload));
				break;
			}
		}
	}

	void Parameter::Write(Stream& stream) const
	{
		stream.WriteUInt(paramaterFlags);
		stream.WriteUInt(interpolationModifiers);
		stream.WriteString(semantic);
		symbol->Write(stream);
	}

	void Parameter::Read(Stream& stream, StringPool& container)
	{
		paramaterFlags = static_cast<ParameterFlags>(stream.ReadUInt());
		interpolationModifiers = static_cast<InterpolationModifier>(stream.ReadUInt());
		semantic = stream.ReadString();
		symbol = make_ast_ptr<SymbolRef>();
		symbol->Read(stream);
	}

	void Parameter::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
	}

	void Field::Write(Stream& stream) const
	{
		stream.WriteUInt(storageClass);
		stream.WriteUInt(interpolationModifiers);
		stream.WriteString(semantic);
		symbol->Write(stream);
	}

	void Field::Read(Stream& stream, StringPool& container)
	{
		storageClass = static_cast<StorageClass>(stream.ReadUInt());
		interpolationModifiers = static_cast<InterpolationModifier>(stream.ReadUInt());
		semantic = stream.ReadString();
		symbol = make_ast_ptr<SymbolRef>();
		symbol->Read(stream);
	}

	void Field::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
	}

	void FunctionOverload::Write(Stream& stream) const
	{
		stream.WriteUInt(functionFlags);
		stream.WriteString(semantic);
		returnSymbol->Write(stream);
	}

	void FunctionOverload::Read(Stream& stream, StringPool& container)
	{
		functionFlags = static_cast<FunctionFlags>(stream.ReadUInt());
		semantic = stream.ReadString();
		returnSymbol = make_ast_ptr<SymbolRef>();
		returnSymbol->Read(stream);
	}

	void FunctionOverload::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
		auto& node = table.GetNode(index);
		for (auto& [span, childIdx] : node.Children)
		{
			auto& child = table.GetNode(childIdx);
			auto& meta = child.Metadata;
			switch (meta->symbolType)
			{
			case SymbolType_Parameter:
				AddParameter(UNIQUE_PTR_CAST_AST(nodes[childIdx], Parameter));
				break;
			}
		}
	}

	const ast_ptr<BlockStatement>& FunctionOverload::GetBody() const noexcept
	{
		return body;
	}

	void FunctionOverload::SetBody(ast_ptr<BlockStatement>&& value) noexcept
	{
		UnregisterChild(body.get()); body = std::move(value); RegisterChild(body.get());
	}

	ast_ptr<BlockStatement>& FunctionOverload::GetBodyMut() noexcept
	{
		return body;
	}

	void ConstructorOverload::Write(Stream& stream) const
	{
		stream.WriteUInt(functionFlags);
		returnSymbol->Write(stream);
	}

	void ConstructorOverload::Read(Stream& stream, StringPool& container)
	{
		functionFlags = static_cast<FunctionFlags>(stream.ReadUInt());
		returnSymbol = make_ast_ptr<SymbolRef>();
		returnSymbol->Read(stream);
	}

	void ConstructorOverload::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
		FunctionOverload::Build(table, index, compilation, nodes);
	}

	void OperatorOverload::Write(Stream& stream) const
	{
		stream.WriteUInt(functionFlags);
		stream.WriteUInt(operatorFlags);
		stream.WriteUInt(_operator);
		returnSymbol->Write(stream);
	}

	void OperatorOverload::Read(Stream& stream, StringPool& container)
	{
		functionFlags = static_cast<FunctionFlags>(stream.ReadUInt());
		operatorFlags = static_cast<OperatorFlags>(stream.ReadUInt());
		_operator = static_cast<Operator>(stream.ReadUInt());
		returnSymbol = make_ast_ptr<SymbolRef>();
		returnSymbol->Read(stream);
	}

	void OperatorOverload::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
	{
		FunctionOverload::Build(table, index, compilation, nodes);
	}

	void Class::Write(Stream& stream) const
	{
	}

	void Class::Read(Stream& stream, StringPool& container)
	{
	}

	void Class::Build(SymbolTable& table, size_t index, CompilationUnit* compilation, std::vector<ast_ptr<SymbolDef>>& nodes)
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
			case SymbolType_Operator:
				AddOperator(UNIQUE_PTR_CAST_AST(nodes[childIdx], OperatorOverload));
				break;
			}
		}
	}
}