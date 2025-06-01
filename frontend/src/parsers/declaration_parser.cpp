#include "declaration_parser.hpp"
#include "sub_parser_registry.hpp"

namespace HXSL
{
	static bool ParseField(Parser& parser, TokenStream& stream, const Token& start, IdentifierInfo* name, ast_ptr<SymbolRef> symbol, TextSpan semantic)
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
		auto field = make_ast_ptr<Field>(span, name, list.accessModifiers, list.storageClasses, list.interpolationModifiers, std::move(symbol), semantic);

		ASTNode* parent = parser.scopeParent();
		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Namespace:
			cast<Namespace>(parent)->AddField(std::move(field));
			break;
		case NodeType_Struct:
			cast<Struct>(parent)->AddField(std::move(field));
			break;
		case NodeType_Class:
			cast<Class>(parent)->AddField(std::move(field));
			break;
		default:
			return false;
		}

		return true;
	}

	static bool ParseParameter(Parser& parser, TokenStream& stream, ast_ptr<Parameter>& parameter)
	{
		auto startingToken = stream.Current();

		auto flags = parser.ParseParameterFlags();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_Type, symbol));
		IdentifierInfo* name;
		IF_ERR_RET_FALSE(stream.ExpectIdentifier(name, EXPECTED_IDENTIFIER));

		TextSpan semantic = {};
		if (stream.TryGetOperator(Operator_Colon))
		{
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic, EXPECTED_IDENTIFIER));
		}

		auto span = startingToken.Span.merge(stream.LastToken().Span);

		parameter = make_ast_ptr<Parameter>(span, name, std::get<0>(flags), std::get<1>(flags), std::move(symbol.make()), semantic);
		return true;
	}

	static bool ParseParameters(Parser& parser, TokenStream& stream, FunctionOverload* overload)
	{
		std::vector<ast_ptr<Parameter>> parameters;
		bool firstParameter = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParameter)
			{
				stream.ExpectDelimiter(',', EXPECTED_COMMA);
			}
			firstParameter = false;

			ast_ptr<Parameter> parameter;

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
		overload->SetParameters(std::move(parameters));

		return true;
	}

	static bool ParseFunction(Parser& parser, TokenStream& stream, const Token& start, IdentifierInfo* name, ast_ptr<SymbolRef> returnSymbol)
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
		auto function = make_ast_ptr<FunctionOverload>(TextSpan(), name, list.accessModifiers, list.functionFlags, std::move(returnSymbol));
		if (attribute && attribute->HasResource())
		{
			function->AddAttribute(std::move(attribute->Take()));
		}

		ParseParameters(parser, stream, function.get());

		if (stream.TryGetOperator(Operator_Colon))
		{
			TextSpan semantic;
			stream.ExpectIdentifier(semantic, EXPECTED_IDENTIFIER);
			function->SetSemantic(semantic.str());
		}

		ast_ptr<BlockStatement> statement;
		ParseStatementBody(ScopeType_Function, parser, stream, statement);
		function->SetBody(std::move(statement));

		function->SetSpan(stream.MakeFromLast(start));

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Namespace:
			cast<Namespace>(parent)->AddFunction(std::move(function));
			break;
		case NodeType_Struct:
			cast<Struct>(parent)->AddFunction(std::move(function));
			break;
		case NodeType_Class:
			cast<Class>(parent)->AddFunction(std::move(function));
			break;
		default:
			return false;
		}

		return true;
	}

	static bool ParseConstructor(Parser& parser, TokenStream& stream, const Token& start, ast_ptr<SymbolRef> returnSymbol)
	{
		TakeHandle<AttributeDeclaration>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_All, true, FunctionFlags_All, StorageClass_None, InterpolationModifier_None, false);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_CTOR);

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			return false;
		}

		auto parent = parser.scopeParent();
		auto ctor = make_ast_ptr<ConstructorOverload>(TextSpan(), parser.GetIdentifierTable().Get("#ctor"), list.accessModifiers, list.functionFlags, std::move(returnSymbol));
		if (attribute && attribute->HasResource())
		{
			ctor->AddAttribute(std::move(attribute->Take()));
		}

		ParseParameters(parser, stream, ctor.get());

		ast_ptr<BlockStatement> statement;
		ParseStatementBody(ScopeType_Function, parser, stream, statement);
		ctor->SetBody(std::move(statement));

		ctor->SetSpan(stream.MakeFromLast(start));

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Struct:
			cast<Struct>(parent)->AddFunction(std::move(ctor));
			break;
		case NodeType_Class:
			cast<Class>(parent)->AddFunction(std::move(ctor));
			break;
		default:
			return false;
		}

		return true;
	}

	static bool ParseOperator(Parser& parser, TokenStream& stream, const Token& start, OperatorFlags flags)
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

		ast_ptr<SymbolRef> symbol;
		IF_ERR_RET_FALSE(parser.ParseSymbol(SymbolRefType_Type, symbol));

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			return false;
		}

		auto name = opKeywordToken.Span.merge(opToken.Span);

		auto _operator = make_ast_ptr<OperatorOverload>(TextSpan(), parser.GetIdentifierTable().Get(name.span()), list.accessModifiers, list.functionFlags, flags, op, std::move(symbol));
		if (attribute && attribute->HasResource())
		{
			_operator->AddAttribute(std::move(attribute->Take()));
		}

		stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);
		ParseParameters(parser, stream, _operator.get());

		if (!stream.TryGetDelimiter(';'))
		{
			ast_ptr<BlockStatement> statement;
			ParseStatementBody(ScopeType_Function, parser, stream, statement);
			_operator->SetBody(std::move(statement));
		}

		_operator->SetSpan(stream.MakeFromLast(start));

		auto parent = parser.scopeParent();
		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Struct:
			cast<Struct>(parent)->AddOperator(std::move(_operator));
			break;
		case NodeType_Class:
			cast<Class>(parent)->AddOperator(std::move(_operator));
			break;
		default:
			return false;
		}

		return true;
	}

	bool DeclarationParser::TryParse(Parser& parser, TokenStream& stream)
	{
		auto startingToken = stream.Current();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_Type, symbol));

		IdentifierInfo* name;
		if (!stream.TryGetIdentifier(name))
		{
			if (stream.TryGetDelimiter('('))
			{
				ParseConstructor(parser, stream, startingToken, std::move(symbol.make()));
				return true;
			}
			return false;
		}

		std::vector<size_t> arraySizes;
		TextSpan fieldSemantic;
		if (stream.TryGetDelimiter('('))
		{
			ParseFunction(parser, stream, startingToken, name, std::move(symbol.make()));
			return true;
		}
		else if (stream.TryGetOperator(Operator_Colon) && stream.TryGetIdentifier(fieldSemantic))
		{
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			ParseField(parser, stream, startingToken, name, std::move(symbol.make()), fieldSemantic);
			return true;
		}
		else if (parser.TryParseArraySizes(arraySizes))
		{
			auto hSymbol = symbol.make(SymbolRefType_ArrayType);
			hSymbol->SetArrayDims(std::move(arraySizes));
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			ParseField(parser, stream, startingToken, name, std::move(hSymbol), {});
		}
		else
		{
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			ParseField(parser, stream, startingToken, name, std::move(symbol.make()), {});
			return true;
		}

		return false;
	}

	bool OperatorParser::TryParse(Parser& parser, TokenStream& stream)
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

		IF_ERR_RET_FALSE(ParseOperator(parser, stream, startingToken, operatorFlags));

		return true;
	}

	bool StructParser::TryParse(Parser& parser, TokenStream& stream)
	{
		auto startingToken = stream.Current();

		IF_ERR_RET_FALSE(stream.TryGetKeyword(Keyword_Struct));

		IdentifierInfo* name;
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

		auto _struct = make_ast_ptr<Struct>(TextSpan(), name, list.accessModifiers, parser.GetIdentifierTable());

		Token t;
		parser.EnterScope(ScopeType_Struct, _struct.get(), t, true, EXPECTED_LEFT_BRACE);
		while (parser.IterateScope(_struct.get()))
		{
			if (!parser.ParseSubStepInner())
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
			cast<Namespace>(parent)->AddStruct(std::move(_struct));
			break;
		case NodeType_Struct:
			cast<Struct>(parent)->AddStruct(std::move(_struct));
			break;
		case NodeType_Class:
			cast<Class>(parent)->AddStruct(std::move(_struct));
			break;
		default:
			return false;
		}

		return true;
	}
}