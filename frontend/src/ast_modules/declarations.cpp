#include "declarations.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	Parameter* Parameter::Create(const TextSpan& span, IdentifierInfo* name, ParameterFlags flags, InterpolationModifier interpolationModifiers, SymbolRef* symbol, IdentifierInfo* semantic)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<Parameter>(sizeof(Parameter), span, name, flags, interpolationModifiers, symbol, semantic);
	}

	FunctionOverload* FunctionOverload::Create(const TextSpan& span,
		IdentifierInfo* name,
		AccessModifier accessModifiers,
		FunctionFlags functionFlags,
		SymbolRef* returnSymbol,
		BlockStatement* body,
		IdentifierInfo* semantic,
		const ArrayRef<Parameter*>& parameters,
		const ArrayRef<AttributeDecl*>& attributes)
	{
		auto* context = ASTContext::GetCurrentContext();
		FunctionOverload* ptr = context->Alloc<FunctionOverload>(TotalSizeToAlloc(parameters.size(), attributes.size()), span, ID, name, accessModifiers, functionFlags, returnSymbol, semantic, body);
		ptr->storage.InitializeMove(ptr, parameters, attributes);
		REGISTER_CHILDREN_PTR(ptr, GetParameters());
		REGISTER_CHILDREN_PTR(ptr, GetAttributes());
		return ptr;
	}

	std::string FunctionOverload::BuildOverloadSignature(bool placeholder) noexcept
	{
		if (!placeholder && !cachedSignature.empty())
		{
			return cachedSignature;
		}

		std::ostringstream oss;
		oss << GetName() << "(";
		bool first = true;
		for (auto& param : GetParameters())
		{
			if (!first)
			{
				oss << ",";
			}
			first = false;
			if (placeholder)
			{
				oss << reinterpret_cast<size_t>(param);
			}
			else
			{
				oss << param->GetSymbolRef()->GetFullyQualifiedName();
			}
		}
		oss << ")";

		if (placeholder)
		{
			return oss.str();
		}
		else
		{
			cachedSignature = oss.str();
			return cachedSignature;
		}
	}

	BlockStatement* FunctionOverload::GetBody() const noexcept
	{
		return body;
	}

	void FunctionOverload::SetBody(BlockStatement* value) noexcept
	{
		UnregisterChild(value); body = value; RegisterChild(value);
	}

	BlockStatement*& FunctionOverload::GetBodyMut() noexcept
	{
		return body;
	}

	void FunctionOverload::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILDREN_MUT(GetAttributes);
		AST_ITERATE_CHILDREN_MUT(GetParameters);
		AST_ITERATE_CHILD_MUT(body);
	}

	void FunctionOverload::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILDREN(GetAttributes);
		AST_ITERATE_CHILDREN(GetParameters);
		AST_ITERATE_CHILD(body);
	}

	ConstructorOverload* ConstructorOverload::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, SymbolRef* targetTypeSymbol, BlockStatement* body, const ArrayRef<Parameter*>& parameters, const ArrayRef<AttributeDecl*>& attributes)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto returnType = SymbolRef::Create(TextSpan(), context->GetIdentifierTable().Get("void"), SymbolRefType_Type, true);
		auto ptr = context->Alloc<ConstructorOverload>(TotalSizeToAlloc(parameters.size(), attributes.size()), span, name, accessModifiers, functionFlags, targetTypeSymbol, returnType, body);
		ptr->storage.InitializeMove(ptr, parameters, attributes);

		return ptr;
	}

	std::string ConstructorOverload::BuildOverloadSignature(bool placeholder) noexcept
	{
		if (!placeholder && !cachedSignature.empty())
		{
			return cachedSignature;
		}

		std::string str = "#ctor(";

		bool first = true;
		for (auto& param : GetParameters())
		{
			if (!first)
			{
				str.push_back(',');
			}
			first = false;

			if (placeholder)
			{
				str.append(std::to_string(reinterpret_cast<size_t>(param)));
			}
			else
			{
				str.append(param->GetSymbolRef()->GetFullyQualifiedName().view());
			}
		}
		str.push_back(')');

		if (placeholder)
		{
			return str;
		}
		else
		{
			cachedSignature = str;
			return cachedSignature;
		}
	}

	OperatorOverload* OperatorOverload::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, SymbolRef* returnSymbol, BlockStatement* body, const ArrayRef<Parameter*>& parameters, const ArrayRef<AttributeDecl*>& attributes)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto ptr = context->Alloc<OperatorOverload>(TotalSizeToAlloc(parameters.size(), attributes.size()), span, name, accessModifiers, functionFlags, operatorFlags, _operator, returnSymbol, body);
		ptr->storage.InitializeMove(ptr, parameters, attributes);
		return ptr;
	}

	std::string OperatorOverload::BuildOverloadSignature(bool placeholder) noexcept
	{
		if (!placeholder && !cachedSignature.empty())
		{
			return cachedSignature;
		}

		std::string str;

		size_t size = 0;

		str.resize(2);
		str[0] = ToLookupChar(_operator);

		if (_operator == Operator_Cast)
		{
			str[1] = '#';
			if (placeholder)
			{
				str.append(std::to_string(reinterpret_cast<size_t>(this)));
			}
			else
			{
				str.append(returnSymbol->GetFullyQualifiedName().view());
			}
			str.push_back('(');
		}
		else
		{
			str[1] = '(';
		}

		bool first = true;
		for (auto& param : GetParameters())
		{
			if (!first)
			{
				str.push_back(',');
			}
			first = false;

			if (placeholder)
			{
				str.append(std::to_string(reinterpret_cast<size_t>(param)));
			}
			else
			{
				str.append(param->GetSymbolRef()->GetFullyQualifiedName().view());
			}
		}
		str.push_back(')');

		if (placeholder)
		{
			return str;
		}
		else
		{
			cachedSignature = str;
			return cachedSignature;
		}
	}

	Field* Field::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, SymbolRef* symbol, IdentifierInfo* semantic)
	{
		auto* context = ASTContext::GetCurrentContext();
		return context->Alloc<Field>(sizeof(Field), span, name, access, storageClass, interpolationModifiers, symbol, semantic);
	}

	ThisDef* ThisDef::Create(IdentifierInfo* parentIdentifier)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto parentType = SymbolRef::Create(TextSpan(), parentIdentifier, SymbolRefType_Type, false);
		return context->Alloc<ThisDef>(sizeof(ThisDef), context->GetIdentifierTable().Get("this"), parentType);
	}

	Struct* Struct::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, const ArrayRef<Field*>& fields, const ArrayRef<Struct*>& structs, const ArrayRef<Class*>& classes, const ArrayRef<ConstructorOverload*>& constructors, const ArrayRef<FunctionOverload*>& functions, const ArrayRef<OperatorOverload*>& operators)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto thisDef = ThisDef::Create(name);
		auto ptr = context->Alloc<Struct>(TotalSizeToAlloc(fields.size(), structs.size(), classes.size(), constructors.size(), functions.size(), operators.size()), span, name, access, thisDef);
		ptr->storage.InitializeMove(ptr, fields, structs, classes, constructors, functions, operators);
		return ptr;
	}

	Struct* Struct::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto thisDef = ThisDef::Create(name);
		auto ptr = context->Alloc<Struct>(TotalSizeToAlloc(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), span, name, access, thisDef);
		ptr->storage.SetCounts(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators);
		return ptr;
	}

	Class* Class::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, const ArrayRef<Field*>& fields, const ArrayRef<Struct*>& structs, const ArrayRef<Class*>& classes, const ArrayRef<ConstructorOverload*>& constructors, const ArrayRef<FunctionOverload*>& functions, const ArrayRef<OperatorOverload*>& operators)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto thisDef = ThisDef::Create(name);
		auto ptr = context->Alloc<Class>(TotalSizeToAlloc(fields.size(), structs.size(), classes.size(), constructors.size(), functions.size(), operators.size()), span, name, access, thisDef);
		ptr->storage.InitializeMove(ptr, fields, structs, classes, constructors, functions, operators);
		return ptr;
	}

	Class* Class::Create(const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators)
	{
		auto* context = ASTContext::GetCurrentContext();
		auto thisDef = ThisDef::Create(name);
		auto ptr = context->Alloc<Class>(TotalSizeToAlloc(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), span, name, access, thisDef);
		ptr->storage.SetCounts(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators);
		return ptr;
	}

	void DataTypeBase::ForEachChild(ASTChildCallback cb, void* userdata)
	{
		AST_ITERATE_CHILD_MUT(thisDef);
		AST_ITERATE_CHILDREN_MUT(GetFields);
		AST_ITERATE_CHILDREN_MUT(GetStructs);
		AST_ITERATE_CHILDREN_MUT(GetClasses);
		AST_ITERATE_CHILDREN_MUT(GetConstructors);
		AST_ITERATE_CHILDREN_MUT(GetFunctions);
		AST_ITERATE_CHILDREN_MUT(GetOperators);
	}

	void DataTypeBase::ForEachChild(ASTConstChildCallback cb, void* userdata) const
	{
		AST_ITERATE_CHILD(thisDef);
		AST_ITERATE_CHILDREN(GetFields);
		AST_ITERATE_CHILDREN(GetStructs);
		AST_ITERATE_CHILDREN(GetClasses);
		AST_ITERATE_CHILDREN(GetConstructors);
		AST_ITERATE_CHILDREN(GetFunctions);
		AST_ITERATE_CHILDREN(GetOperators);
	}

	/*
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
	}*/
}