#include "declaration_parser.hpp"
#include "sub_parser_registry.hpp"

namespace HXSL
{
	static bool ParseField(Parser& parser, TokenStream& stream, const Token& start, IdentifierInfo* name, SymbolRef* symbol, IdentifierInfo* semantic, Field*& fieldOut)
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
		auto field = Field::Create(span, name, list.accessModifiers, list.storageClasses, list.interpolationModifiers, symbol, semantic);

		fieldOut = field;

		return true;
	}

	static bool ParseParameter(Parser& parser, TokenStream& stream, Parameter*& parameter)
	{
		auto startingToken = stream.Current();

		auto flags = parser.ParseParameterFlags();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_Type, symbol));
		IdentifierInfo* name;
		IF_ERR_RET_FALSE(stream.ExpectIdentifier(name, EXPECTED_IDENTIFIER));

		IdentifierInfo* semantic = nullptr;
		if (stream.TryGetOperator(Operator_Colon))
		{
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic, EXPECTED_IDENTIFIER));
		}

		auto span = startingToken.Span.merge(stream.LastToken().Span);

		parameter = Parameter::Create(span, name, std::get<0>(flags), std::get<1>(flags), symbol.make(), semantic);
		return true;
	}

	static bool ParseParameters(Parser& parser, TokenStream& stream, std::vector<Parameter*>& parameters)
	{
		bool firstParameter = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParameter)
			{
				stream.ExpectDelimiter(',', EXPECTED_COMMA);
			}
			firstParameter = false;

			Parameter* parameter;

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

		return true;
	}

	static bool ParseFunction(Parser& parser, TokenStream& stream, const Token& start, IdentifierInfo* name, SymbolRef* returnSymbol, FunctionOverload*& function)
	{
		TakeHandle<AttributeDecl>* attribute = nullptr;
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

		std::vector<Parameter*> parameters;
		ParseParameters(parser, stream, parameters);

		IdentifierInfo* semantic = nullptr;
		if (stream.TryGetOperator(Operator_Colon))
		{
			stream.ExpectIdentifier(semantic, EXPECTED_IDENTIFIER);
		}

		BlockStatement* statement;
		ParseStatementBody(ScopeType_Function, parser, stream, statement);

		auto span = stream.MakeFromLast(start);
		std::vector<AttributeDecl*> attributes;
		if (attribute && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}

		function = FunctionOverload::Create(span, name, list.accessModifiers, list.functionFlags, returnSymbol, statement, semantic, parameters, attributes);
		return true;
	}

	static bool ParseConstructor(Parser& parser, TokenStream& stream, const Token& start, SymbolRef* returnSymbol, ConstructorOverload*& ctor)
	{
		TakeHandle<AttributeDecl>* attribute = nullptr;
		parser.AcceptAttribute(&attribute, 0);

		ModifierList list;
		ModifierList allowed = ModifierList(AccessModifier_All, true, FunctionFlags_All, StorageClass_None, InterpolationModifier_None, false);
		parser.AcceptModifierList(&list, allowed, INVALID_MODIFIER_ON_CTOR);

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			return false;
		}

		std::vector<AttributeDecl*> attributes;
		if (attribute && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}

		std::vector<Parameter*> parameters;
		ParseParameters(parser, stream, parameters);

		BlockStatement* statement;
		ParseStatementBody(ScopeType_Function, parser, stream, statement);

		auto span = stream.MakeFromLast(start);
		ctor = ConstructorOverload::Create(span, parser.GetIdentifierTable().Get("#ctor"), list.accessModifiers, list.functionFlags, returnSymbol, statement, parameters, attributes);
		return true;
	}

	static bool ParseOperator(Parser& parser, TokenStream& stream, const Token& start, OperatorFlags flags, OperatorOverload*& _operator)
	{
		auto opKeywordToken = stream.Current();
		stream.ExpectKeyword(Keyword_Operator, EXPECTED_OPERATOR);

		TakeHandle<AttributeDecl>* attribute = nullptr;
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

		SymbolRef* symbol;
		IF_ERR_RET_FALSE(parser.ParseSymbol(SymbolRefType_Type, symbol));

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Struct && scopeType != ScopeType_Class)
		{
			return false;
		}

		auto name = opKeywordToken.Span.merge(opToken.Span);

		std::vector<AttributeDecl*> attributes;
		if (attribute && attribute->HasResource())
		{
			attributes.push_back(attribute->Take());
		}

		stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);

		std::vector<Parameter*> parameters;
		ParseParameters(parser, stream, parameters);

		BlockStatement* statement;
		ParseStatementBody(ScopeType_Function, parser, stream, statement);

		auto span = stream.MakeFromLast(start);
		_operator = OperatorOverload::Create(span, parser.GetIdentifierTable().Get(name.span()), list.accessModifiers, list.functionFlags, flags, op, symbol, statement, parameters, attributes);
		return true;
	}

	static void ParseDeclScope(Parser& parser, DeclContainerBuilder& builder, ScopeType scopeType)
	{
		Token t;
		parser.EnterScope(scopeType, nullptr, t, true, EXPECTED_LEFT_BRACE);
		while (parser.IterateScope(nullptr))
		{
			ASTNode* decl;
			if (!parser.ParseSubStepInner(decl))
			{
				if (!parser.TryRecoverScope(nullptr, true))
				{
					break;
				}
			}
			else
			{
				builder.AddDeclaration(decl);
			}
		}
	}

	bool NamespaceParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_Namespace))
		{
			return false;
		}

		parser.RejectAttribute(ATTRIBUTE_INVALID_IN_CONTEXT);
		parser.RejectModifierList(INVALID_MODIFIER_ON_NAMESPACE);

		IdentifierInfo* name;
		stream.ExpectIdentifier(name, EXPECTED_IDENTIFIER);

		DeclContainerBuilder builder = DeclContainerBuilder(parser.GetLogger(), DeclContainerFlags::AllowPresetNamespace);
		std::vector<UsingDecl*> usings;
		if (stream.TryGetDelimiter(';'))
		{
			while (stream.CanAdvance())
			{
				ASTNode* decl;
				if (parser.ParseSubStepInner(decl))
				{
					builder.AddDeclaration(decl);
				}
			}
		}
		else
		{
			ParseDeclScope(parser, builder, ScopeType_Namespace);
		}
		
		auto span = stream.MakeFromLast(start);
		auto ns = Namespace::Create(span, name, builder.structs, builder.classes, builder.functions, builder.fields, builder.namespaces, builder.usings);
		declOut = ns;
		return true;
	}

	bool UsingParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut)
	{
		auto start = stream.Current();
		if (!stream.TryGetKeyword(Keyword_Using))
		{
			return false;
		}

		bool hasDot;
		auto identifier = parser.ParseQualifiedName(hasDot);
		IdentifierInfo* target;
		IdentifierInfo* alias = nullptr;
		if (hasDot)
		{
			target = identifier;
		}
		else if (stream.TryGetOperator(Operator_Equal))
		{
			alias = identifier;
			target = parser.ParseQualifiedName(hasDot);
		}
		else
		{
			target = identifier;
		}
		stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
		auto span = stream.MakeFromLast(start);
		declOut = UsingDecl::Create(span, target, alias);
		return true;
	}

	bool DeclarationParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut)
	{
		auto startingToken = stream.Current();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_Type, symbol));

		IdentifierInfo* name;
		if (!stream.TryGetIdentifier(name))
		{
			if (stream.TryGetDelimiter('('))
			{
				ConstructorOverload* ctor;
				ParseConstructor(parser, stream, startingToken, symbol.make(), ctor);
				declOut = ctor;
				return true;
			}
			return false;
		}

		std::vector<size_t> arraySizes;
		IdentifierInfo* fieldSemantic;
		if (stream.TryGetDelimiter('('))
		{
			FunctionOverload* function;
			ParseFunction(parser, stream, startingToken, name, symbol.make(), function);
			declOut = function;
			return true;
		}
		else if (stream.TryGetOperator(Operator_Colon) && stream.TryGetIdentifier(fieldSemantic))
		{
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			Field* field;
			ParseField(parser, stream, startingToken, name, symbol.make(), fieldSemantic, field);
			return true;
		}
		else if (parser.TryParseArraySizes(arraySizes))
		{
			auto hSymbol = symbol.MakeSpan(arraySizes);
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			Field* field;
			ParseField(parser, stream, startingToken, name, hSymbol, {}, field);
			declOut = field;
			return true;
		}
		else
		{
			stream.ExpectDelimiter(';', EXPECTED_SEMICOLON);
			Field* field;
			ParseField(parser, stream, startingToken, name, symbol.make(), {}, field);
			declOut = field;
			return true;
		}

		return false;
	}

	bool OperatorParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut)
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

		OperatorOverload* _operator;
		IF_ERR_RET_FALSE(ParseOperator(parser, stream, startingToken, operatorFlags, _operator));
		declOut = _operator;

		return true;
	}

	bool StructParser::TryParse(Parser& parser, TokenStream& stream, ASTNode*& declOut)
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

		DeclContainerBuilder builder = DeclContainerBuilder(parser.GetLogger(), DeclContainerFlags::AllowPresetStruct);
		ParseDeclScope(parser, builder, ScopeType_Struct);
		stream.TryGetDelimiter(';'); // this is optional

		auto span = stream.MakeFromLast(startingToken);
		auto _struct = Struct::Create(span, name, list.accessModifiers, builder.fields, builder.structs, builder.classes, builder.constructors, builder.functions, builder.operators);
		declOut = _struct;
		return true;
	}
}