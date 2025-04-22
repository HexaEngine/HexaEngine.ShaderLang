#include "declaration_parser.hpp"
#include "sub_parser_registry.hpp"

#include <memory>

namespace HXSL
{
	static bool ParseField(const Token& start, AccessModifier access, FieldFlags flags, TextSpan name, std::unique_ptr<SymbolRef> symbol, TextSpan semantic, Parser& parser, TokenStream& stream, Compilation* compilation)
	{
		parser.RejectAttribute("cannot be applied to fields, found on '%s'", name.toString().c_str());
		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct)
		{
			ERR_RETURN_FALSE(parser, "Cannot declare field in this scope");
		}

		ASTNode* parent = parser.scopeParent();
		auto span = start.Span.merge(stream.LastToken().Span);
		auto field = std::make_unique<Field>(span, parent, access, flags, name, std::move(symbol), semantic);

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Compilation:
			compilation->AddField(std::move(field));
			break;
		case NodeType_Namespace:
			parent->As<Namespace>()->AddField(std::move(field));
			break;
		case NodeType_Struct:
			parent->As<Struct>()->AddField(std::move(field));
			break;
		default:
			return false;
		}

		return true;
	}

	static bool ParseParameter(Parser& parser, TokenStream& stream, ASTNode* parent, std::unique_ptr<Parameter>& parameter)
	{
		auto startingToken = stream.Current();

		ParameterFlags flags = parser.ParseParameterFlags();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_AnyType, symbol));
		TextSpan name;
		IF_ERR_RET_FALSE(stream.ExpectIdentifier(name));

		TextSpan semantic = {};
		if (stream.TryGetOperator(Operator_Colon))
		{
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic));
		}

		auto span = startingToken.Span.merge(stream.LastToken().Span);

		parameter = std::make_unique<Parameter>(span, parent, flags, std::move(symbol.make()), name, semantic);
		return true;
	}

	static bool ParseFunction(const Token& start, AccessModifier access, FunctionFlags flags, TextSpan name, std::unique_ptr<SymbolRef> returnSymbol, Parser& parser, TokenStream& stream, Compilation* compilation, TakeHandle<AttributeDeclaration>* attribute)
	{
		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct)
		{
			return false;
		}

		auto parent = parser.scopeParent();
		auto function = std::make_unique<Function>(TextSpan(), parent, access, flags, name, std::move(returnSymbol));
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
				stream.ExpectDelimiter(',');
			}
			firstParameter = false;

			std::unique_ptr<Parameter> parameter;

			IF_ERR_RET_FALSE(ParseParameter(parser, stream, function.get(), parameter));

			parameters.push_back(std::move(parameter));
		}

		function->SetParameters(std::move(parameters));

		if (stream.TryGetOperator(Operator_Colon))
		{
			TextSpan semantic;
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic));
			function->SetSemantic(semantic);
		}

		if (!stream.TryGetDelimiter(';'))
		{
			std::unique_ptr<BlockStatement> statement;
			IF_ERR_RET_FALSE(ParseStatementBody(name, ScopeType_Function, function.get(), parser, stream, statement));
			function->SetBody(std::move(statement));
		}

		function->SetSpan(stream.MakeFromLast(start));

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Compilation:
			compilation->AddFunction(std::move(function));
			break;
		case NodeType_Namespace:
			parent->As<Namespace>()->AddFunction(std::move(function));
			break;
		case NodeType_Struct:
			parent->As<Struct>()->AddFunction(std::move(function));
			break;
		default:
			return false;
		}

		return true;
	}

	bool DeclarationParser::TryParse(Parser& parser, TokenStream& stream, Compilation* compilation)
	{
		auto startingToken = stream.Current();

		AccessModifier access = parser.ParseAccessModifier();
		FunctionFlags functionFlags = parser.ParseFunctionFlags();
		FieldFlags fieldFlags = parser.ParseFieldFlags();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(SymbolRefType_AnyType, symbol));

		TextSpan name;
		IF_ERR_RET_FALSE(stream.TryGetIdentifier(name));

		TextSpan fieldSemantic;
		if (stream.TryGetDelimiter('('))
		{
			TakeHandle<AttributeDeclaration>* attribute = nullptr;
			parser.AcceptAttribute(&attribute, "");
			IF_ERR_RET_FALSE(ParseFunction(startingToken, access, functionFlags, name, std::move(symbol.make()), parser, stream, compilation, attribute));
			return true;
		}
		else if (stream.TryGetDelimiter(';'))
		{
			ParseField(startingToken, access, fieldFlags, name, std::move(symbol.make()), {}, parser, stream, compilation);
			return true;
		}
		else if (stream.TryGetOperator(Operator_Colon) && stream.TryGetIdentifier(fieldSemantic) && stream.TryGetDelimiter(';'))
		{
			ParseField(startingToken, access, fieldFlags, name, std::move(symbol.make()), fieldSemantic, parser, stream, compilation);
			return true;
		}

		return false;
	}

	bool StructParser::TryParse(Parser& parser, TokenStream& stream, Compilation* compilation)
	{
		auto startingToken = stream.Current();

		auto access = parser.ParseAccessModifier();

		IF_ERR_RET_FALSE(stream.TryGetKeyword(Keyword_Struct));

		TextSpan name;
		IF_ERR_RET_FALSE(stream.ExpectIdentifier(name));

		IF_ERR_RET_FALSE(parser.AcceptAttribute(nullptr, "is not valid in this context on '%s'", name.toString().c_str()));

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct)
		{
			ERR_RETURN_FALSE(parser, "Cannot declare struct here.");
		}

		auto parent = parser.scopeParent();
		auto _struct = std::make_unique<Struct>(TextSpan(), parent, access, name);

		Token t;
		IF_ERR_RET_FALSE(parser.EnterScope(name, ScopeType_Struct, _struct.get(), t));
		while (parser.IterateScope())
		{
			while (SubParserRegistry::TryParse(parser, stream, _struct.get(), compilation));
		}

		stream.TryGetDelimiter(';'); // this is optional

		auto span = startingToken.Span.merge(stream.LastToken().Span);
		_struct->SetSpan(span);

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case NodeType_Compilation:
			compilation->AddStruct(std::move(_struct));
			break;
		case NodeType_Namespace:
			parent->As<Namespace>()->AddStruct(std::move(_struct));
			break;
		case NodeType_Struct:
			parent->As<Struct>()->AddStruct(std::move(_struct));
			break;
		default:
			return false;
		}

		return true;
	}
}