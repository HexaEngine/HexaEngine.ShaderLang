#include "declaration_parser.hpp"
#include "sub_parser_registry.hpp"

namespace HXSL
{
	static bool ParseField(const Token& start, TextSpan name, std::unique_ptr<SymbolRef> symbol, TextSpan semantic, Parser& parser, TokenStream& stream, CompilationUnit* compilation)
	{
		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_All, true, FunctionFlags_None, StorageClass_All, InterpolationModifier_All, true);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_FIELD);

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			ERR_RETURN_FALSE(parser, FIELD_DECL_OUT_OF_SCOPE);
		}

		auto span = start.Span.merge(stream.LastToken().Span);
		auto field = std::make_unique<Field>(span, list.accessModifiers, list.storageClasses, list.interpolationModifiers, name, std::move(symbol), semantic);

		ASTNode* parent = parser.scopeParent();
		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Namespace:
			parent->As<Namespace>()->AddField(std::move(field));
			break;
		case NodeType_Struct:
			parent->As<Struct>()->AddField(std::move(field));
			break;
		case NodeType_Class:
			parent->As<Class>()->AddField(std::move(field));
			break;
		default:
			return false;
		}

		return true;
	}

	static bool ParseParameter(Parser& parser, TokenStream& stream, std::unique_ptr<Parameter>& parameter)
	{
		auto startingToken = stream.Current();

		auto flags = parser.ParseParameterFlags();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_Type, symbol));
		TextSpan name;
		IF_ERR_RET_FALSE(stream.ExpectIdentifier(name, EXPECTED_IDENTIFIER));

		TextSpan semantic = {};
		if (stream.TryGetOperator(Operator_Colon))
		{
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic, EXPECTED_IDENTIFIER));
		}

		auto span = startingToken.Span.merge(stream.LastToken().Span);

		parameter = std::make_unique<Parameter>(span, std::get<0>(flags), std::get<1>(flags), std::move(symbol.make()), name, semantic);
		return true;
	}

	static bool ParseFunction(const Token& start, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol, Parser& parser, TokenStream& stream, CompilationUnit* compilation)
	{
		TakeHandle<AttributeDeclaration>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_All, true, FunctionFlags_All, StorageClass_Static, InterpolationModifier_None, false);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_FUNC);

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			return false;
		}

		auto parent = parser.scopeParent();
		auto function = std::make_unique<FunctionOverload>(TextSpan(), list.accessModifiers, list.functionFlags, name, std::move(returnSymbol));
		if (attribute && attribute->HasResource())
		{
			function->AddAttribute(std::move(attribute->Take()));
		}

		std::vector<std::unique_ptr<Parameter>> parameters;

		bool firstParameter = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParameter)
			{
				stream.ExpectDelimiter(',', EXPECTED_COMMA);
			}
			firstParameter = false;

			std::unique_ptr<Parameter> parameter;

			IF_ERR_RET_FALSE(ParseParameter(parser, stream, parameter));

			parameters.push_back(std::move(parameter));
		}

		function->SetParameters(std::move(parameters));

		if (stream.TryGetOperator(Operator_Colon))
		{
			TextSpan semantic;
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic, EXPECTED_IDENTIFIER));
			function->SetSemantic(semantic.str());
		}

		if (!stream.TryGetDelimiter(';'))
		{
			std::unique_ptr<BlockStatement> statement;
			IF_ERR_RET_FALSE(ParseStatementBody(ScopeType_Function, parser, stream, statement));
			function->SetBody(std::move(statement));
		}

		function->SetSpan(stream.MakeFromLast(start));

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Namespace:
			parent->As<Namespace>()->AddFunction(std::move(function));
			break;
		case NodeType_Struct:
			parent->As<Struct>()->AddFunction(std::move(function));
			break;
		case NodeType_Class:
			parent->As<Class>()->AddFunction(std::move(function));
			break;
		default:
			return false;
		}

		return true;
	}

	static bool ParseOperator(const Token& start, OperatorFlags flags, Parser& parser, TokenStream& stream, CompilationUnit* compilation)
	{
		auto opKeywordToken = stream.Current();
		stream.ExpectKeyword(Keyword_Operator, EXPECTED_OPERATOR);

		TakeHandle<AttributeDeclaration>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_All, true, FunctionFlags_Inline, StorageClass_Static, InterpolationModifier_None, false);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_OP);

		Operator op;
		auto opToken = stream.Current();
		if (opToken.isOperator(op))
		{
			stream.Advance();
		}
		else
		{
			opToken = opKeywordToken;
			op = Operator_Cast;
		}

		std::unique_ptr<SymbolRef> symbol;
		IF_ERR_RET_FALSE(parser.ParseSymbol(SymbolRefType_Type, symbol));

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			return false;
		}

		auto name = opKeywordToken.Span.merge(opToken.Span);

		auto _operator = std::make_unique<OperatorOverload>(TextSpan(), list.accessModifiers, list.functionFlags, flags, name, op, std::move(symbol));
		if (attribute && attribute->HasResource())
		{
			_operator->AddAttribute(std::move(attribute->Take()));
		}

		std::vector<std::unique_ptr<Parameter>> parameters;

		stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);

		bool firstParameter = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParameter)
			{
				stream.ExpectDelimiter(',', EXPECTED_COMMA);
			}
			firstParameter = false;

			std::unique_ptr<Parameter> parameter;

			if (ParseParameter(parser, stream, parameter))
			{
				parameters.push_back(std::move(parameter));
			}
			else
			{
				if (!parser.TryRecoverParameterList())
				{
					break;
				}
			}
		}

		_operator->SetParameters(std::move(parameters));

		if (!stream.TryGetDelimiter(';'))
		{
			std::unique_ptr<BlockStatement> statement;
			ParseStatementBody(ScopeType_Function, parser, stream, statement);
			_operator->SetBody(std::move(statement));
		}

		_operator->SetSpan(stream.MakeFromLast(start));

		auto parent = parser.scopeParent();
		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Struct:
			parent->As<Struct>()->AddOperator(std::move(_operator));
			break;
		case NodeType_Class:
			parent->As<Class>()->AddOperator(std::move(_operator));
			break;
		default:
			return false;
		}

		return true;
	}

	bool DeclarationParser::TryParse(Parser& parser, TokenStream& stream, CompilationUnit* compilation)
	{
		auto startingToken = stream.Current();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_Type, symbol));

		TextSpan name;
		if (!stream.TryGetIdentifier(name))
		{
			return false;
		}

		std::vector<size_t> arraySizes;
		TextSpan fieldSemantic;
		if (stream.TryGetDelimiter('('))
		{
			ParseFunction(startingToken, name, std::move(symbol.make()), parser, stream, compilation);
			return true;
		}
		else if (stream.TryGetOperator(Operator_Colon) && stream.TryGetIdentifier(fieldSemantic))
		{
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			ParseField(startingToken, name, std::move(symbol.make()), fieldSemantic, parser, stream, compilation);
			return true;
		}
		else if (parser.TryParseArraySizes(arraySizes))
		{
			auto hSymbol = symbol.make(SymbolRefType_ArrayType);
			hSymbol->SetArrayDims(std::move(arraySizes));
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			ParseField(startingToken, name, std::move(hSymbol), {}, parser, stream, compilation);
		}
		else
		{
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			ParseField(startingToken, name, std::move(symbol.make()), {}, parser, stream, compilation);
			return true;
		}

		return false;
	}

	bool OperatorParser::TryParse(Parser& parser, TokenStream& stream, CompilationUnit* compilation)
	{
		auto startingToken = stream.Current();

		if (!stream.TryGetKeyword(Keyword_Explicit) && !stream.TryGetKeyword(Keyword_Implicit) && !startingToken.isKeywordOf(Keyword_Operator))
		{
			return false;
		}

		OperatorFlags operatorFlags;
		switch (startingToken.asKeyword())
		{
		case Keyword_Explicit:
			operatorFlags = OperatorFlags_Explicit;
			break;
		case Keyword_Implicit:
			operatorFlags = OperatorFlags_Implicit;
			break;
		default:
			operatorFlags = OperatorFlags_None;
			break;
		}

		IF_ERR_RET_FALSE(ParseOperator(startingToken, operatorFlags, parser, stream, compilation));

		return true;
	}

	bool StructParser::TryParse(Parser& parser, TokenStream& stream, CompilationUnit* compilation)
	{
		auto startingToken = stream.Current();

		IF_ERR_RET_FALSE(stream.TryGetKeyword(Keyword_Struct));

		TextSpan name;
		stream.ExpectIdentifier(name, EXPECTED_IDENTIFIER);

		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			parser.Log(STRUCT_DECL_OUT_OF_SCOPE, startingToken);
		}

		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_All, true);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_STRUCT);

		auto _struct = std::make_unique<Struct>(TextSpan(), list.accessModifiers, name);

		Token t;
		parser.EnterScope(ScopeType_Struct, _struct.get(), t, true, EXPECTED_LEFT_BRACE);
		while (parser.IterateScope(_struct.get()))
		{
			if (!parser.ParseSubStepInner(_struct.get()))
			{
				if (!parser.TryRecoverScope(_struct.get(), true))
				{
					break;
				}
			}
		}

		stream.TryGetDelimiter(';'); // this is optional

		auto span = startingToken.Span.merge(stream.LastToken().Span);
		_struct->SetSpan(span);

		auto parent = parser.scopeParent();
		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Namespace:
			parent->As<Namespace>()->AddStruct(std::move(_struct));
			break;
		case NodeType_Struct:
			parent->As<Struct>()->AddStruct(std::move(_struct));
			break;
		case NodeType_Class:
			parent->As<Class>()->AddStruct(std::move(_struct));
			break;
		default:
			return false;
		}

		return true;
	}
}