#include "declarations.hpp"
#include "semantics/symbols/symbol_table.hpp"
#include "ast_context.hpp"

namespace HXSL
{
	Parameter* Parameter::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, ParameterFlags flags, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef>&& symbol, IdentifierInfo* semantic)
	{
		return context->Alloc<Parameter>(sizeof(Parameter), span, name, flags, interpolationModifiers, std::move(symbol), semantic);
	}

	FunctionOverload* FunctionOverload::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& returnSymbol, ArrayRef<ast_ptr<Parameter>>& parameters)
	{
		FunctionOverload* ptr = context->Alloc<FunctionOverload>(TotalSizeToAlloc(parameters.size()), span, ID, name, accessModifiers, functionFlags, std::move(returnSymbol));
		ptr->numParameters = static_cast<uint32_t>(parameters.size());
		std::uninitialized_move(parameters.begin(), parameters.end(), ptr->GetParameters().data());
		return ptr;
	}

	FunctionOverload* FunctionOverload::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& returnSymbol, uint32_t numParameters)
	{
		FunctionOverload* ptr = context->Alloc<FunctionOverload>(TotalSizeToAlloc(numParameters), span, ID, name, accessModifiers, functionFlags, std::move(returnSymbol));
		ptr->numParameters = numParameters;
		ptr->GetParameters().init();
		return ptr;
	}

	std::string FunctionOverload::BuildOverloadSignature(bool placeholder) noexcept
	{
		if (!placeholder && !cachedSignature.empty())
		{
			return cachedSignature;
		}

		std::ostringstream oss;
		oss << name << "(";
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
				oss << reinterpret_cast<size_t>(param.get());
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

	ConstructorOverload* ConstructorOverload::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& targetTypeSymbol, ArrayRef<ast_ptr<Parameter>>& parameters)
	{
		auto returnType = SymbolRef::Create(context, TextSpan(), context->GetIdentiferTable().Get("void"), SymbolRefType_Type, true);
		auto ptr = context->Alloc<ConstructorOverload>(TotalSizeToAlloc(parameters.size()), span, name, accessModifiers, functionFlags, std::move(targetTypeSymbol), ast_ptr<SymbolRef>(returnType));
		ptr->numParameters = static_cast<uint32_t>(parameters.size());
		std::uninitialized_move(parameters.begin(), parameters.end(), ptr->GetParameters().data());
		return ptr;
	}

	ConstructorOverload* ConstructorOverload::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, ast_ptr<SymbolRef>&& targetTypeSymbol, uint32_t numParameters)
	{
		auto returnType = SymbolRef::Create(context, TextSpan(), context->GetIdentiferTable().Get("void"), SymbolRefType_Type, true);
		auto ptr = context->Alloc<ConstructorOverload>(TotalSizeToAlloc(numParameters), span, name, accessModifiers, functionFlags, std::move(targetTypeSymbol), ast_ptr<SymbolRef>(returnType));
		ptr->numParameters = numParameters;
		ptr->GetParameters().init();
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
				str.append(std::to_string(reinterpret_cast<size_t>(param.get())));
			}
			else
			{
				str.append(param->GetSymbolRef()->GetFullyQualifiedName());
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

	OperatorOverload* OperatorOverload::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, ast_ptr<SymbolRef>&& returnSymbol, ArrayRef<ast_ptr<Parameter>>& parameters)
	{
		auto ptr = context->Alloc<OperatorOverload>(TotalSizeToAlloc(parameters.size()), span, name, accessModifiers, functionFlags, operatorFlags, _operator, std::move(returnSymbol));
		ptr->numParameters = static_cast<uint32_t>(parameters.size());
		std::uninitialized_move(parameters.begin(), parameters.end(), ptr->GetParameters().data());
		return ptr;
	}

	OperatorOverload* OperatorOverload::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier accessModifiers, FunctionFlags functionFlags, OperatorFlags operatorFlags, Operator _operator, ast_ptr<SymbolRef>&& returnSymbol, uint32_t numParameters)
	{
		auto ptr = context->Alloc<OperatorOverload>(TotalSizeToAlloc(numParameters), span, name, accessModifiers, functionFlags, operatorFlags, _operator, std::move(returnSymbol));
		ptr->numParameters = numParameters;
		ptr->GetParameters().init();
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
				str.append(returnSymbol->GetFullyQualifiedName());
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
				str.append(std::to_string(reinterpret_cast<size_t>(param.get())));
			}
			else
			{
				str.append(param->GetSymbolRef()->GetFullyQualifiedName());
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

	Field* Field::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, StorageClass storageClass, InterpolationModifier interpolationModifiers, ast_ptr<SymbolRef>&& symbol, IdentifierInfo* semantic)
	{
		return context->Alloc<Field>(sizeof(Field), span, name, access, storageClass, interpolationModifiers, std::move(symbol), semantic);
	}

	ThisDef* ThisDef::Create(ASTContext* context, IdentifierInfo* parentIdentifier)
	{
		auto parentType = ast_ptr<SymbolRef>(SymbolRef::Create(context, TextSpan(), parentIdentifier, SymbolRefType_Type, false));
		return context->Alloc<ThisDef>(sizeof(ThisDef), context->GetIdentiferTable().Get("this"), std::move(parentType));
	}

	Struct* Struct::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, ArrayRef<ast_ptr<Field>>& fields, ArrayRef<ast_ptr<Struct>>& structs, ArrayRef<ast_ptr<Class>>& classes, ArrayRef<ast_ptr<ConstructorOverload>>& constructors, ArrayRef<ast_ptr<FunctionOverload>>& functions, ArrayRef<ast_ptr<OperatorOverload>>& operators)
	{
		auto thisDef = ast_ptr<ThisDef>(ThisDef::Create(context, name));
		auto ptr = context->Alloc<Struct>(TotalSizeToAlloc(fields.size(), structs.size(), classes.size(), constructors.size(), functions.size(), operators.size()), span, name, access, std::move(thisDef));
		ptr->numFields = static_cast<uint32_t>(fields.size());
		ptr->numStructs = static_cast<uint32_t>(structs.size());
		ptr->numClasses = static_cast<uint32_t>(classes.size());
		ptr->numConstructors = static_cast<uint32_t>(constructors.size());
		ptr->numFunctions = static_cast<uint32_t>(functions.size());
		ptr->numOperators = static_cast<uint32_t>(operators.size());
		std::uninitialized_move(fields.begin(), fields.end(), ptr->GetFields().data());
		std::uninitialized_move(structs.begin(), structs.end(), ptr->GetStructs().data());
		std::uninitialized_move(classes.begin(), classes.end(), ptr->GetClasses().data());
		std::uninitialized_move(constructors.begin(), constructors.end(), ptr->GetConstructors().data());
		std::uninitialized_move(functions.begin(), functions.end(), ptr->GetFunctions().data());
		std::uninitialized_move(operators.begin(), operators.end(), ptr->GetOperators().data());
		return ptr;
	}

	Struct* Struct::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators)
	{
		auto thisDef = ast_ptr<ThisDef>(ThisDef::Create(context, name));
		auto ptr = context->Alloc<Struct>(TotalSizeToAlloc(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), span, name, access, std::move(thisDef));
		ptr->numFields = numFields;
		ptr->numStructs = numStructs;
		ptr->numClasses = numClasses;
		ptr->numConstructors = numConstructors;
		ptr->numFunctions = numFunctions;
		ptr->numOperators = numOperators;
		ptr->GetFields().init();
		ptr->GetStructs().init();
		ptr->GetClasses().init();
		ptr->GetConstructors().init();
		ptr->GetFunctions().init();
		ptr->GetOperators().init();
		return ptr;
	}

	Class* Class::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, ArrayRef<ast_ptr<Field>>& fields, ArrayRef<ast_ptr<Struct>>& structs, ArrayRef<ast_ptr<Class>>& classes, ArrayRef<ast_ptr<ConstructorOverload>>& constructors, ArrayRef<ast_ptr<FunctionOverload>>& functions, ArrayRef<ast_ptr<OperatorOverload>>& operators)
	{
		auto thisDef = ast_ptr<ThisDef>(ThisDef::Create(context, name));
		auto ptr = context->Alloc<Class>(TotalSizeToAlloc(fields.size(), structs.size(), classes.size(), constructors.size(), functions.size(), operators.size()), span, name, access, std::move(thisDef));
		ptr->numFields = static_cast<uint32_t>(fields.size());
		ptr->numStructs = static_cast<uint32_t>(structs.size());
		ptr->numClasses = static_cast<uint32_t>(classes.size());
		ptr->numConstructors = static_cast<uint32_t>(constructors.size());
		ptr->numFunctions = static_cast<uint32_t>(functions.size());
		ptr->numOperators = static_cast<uint32_t>(operators.size());
		std::uninitialized_move(fields.begin(), fields.end(), ptr->GetFields().data());
		std::uninitialized_move(structs.begin(), structs.end(), ptr->GetStructs().data());
		std::uninitialized_move(classes.begin(), classes.end(), ptr->GetClasses().data());
		std::uninitialized_move(constructors.begin(), constructors.end(), ptr->GetConstructors().data());
		std::uninitialized_move(functions.begin(), functions.end(), ptr->GetFunctions().data());
		std::uninitialized_move(operators.begin(), operators.end(), ptr->GetOperators().data());
		return ptr;
	}

	Class* Class::Create(ASTContext* context, const TextSpan& span, IdentifierInfo* name, AccessModifier access, uint32_t numFields, uint32_t numStructs, uint32_t numClasses, uint32_t numConstructors, uint32_t numFunctions, uint32_t numOperators)
	{
		auto thisDef = ast_ptr<ThisDef>(ThisDef::Create(context, name));
		auto ptr = context->Alloc<Class>(TotalSizeToAlloc(numFields, numStructs, numClasses, numConstructors, numFunctions, numOperators), span, name, access, std::move(thisDef));
		ptr->numFields = numFields;
		ptr->numStructs = numStructs;
		ptr->numClasses = numClasses;
		ptr->numConstructors = numConstructors;
		ptr->numFunctions = numFunctions;
		ptr->numOperators = numOperators;
		ptr->GetFields().init();
		ptr->GetStructs().init();
		ptr->GetClasses().init();
		ptr->GetConstructors().init();
		ptr->GetFunctions().init();
		ptr->GetOperators().init();
		return ptr;
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