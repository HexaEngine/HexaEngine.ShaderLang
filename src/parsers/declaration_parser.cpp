#include "declaration_parser.hpp"
#include "sub_parser_registry.hpp"

#include <memory>
using namespace std;

namespace HXSL
{
	static bool ParseField(const Token& start, HXSLAccessModifier access, HXSLFieldFlags flags, TextSpan name, unique_ptr<HXSLSymbolRef> symbol, TextSpan semantic, HXSLParser& parser, TokenStream& stream, HXSLCompilation* compilation)
	{
		parser.RejectAttribute("cannot be applied to fields, found on '%s'", name.toString().c_str());
		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct)
		{
			ERR_RETURN_FALSE(parser, "Cannot declare field in this scope");
		}

		HXSLNode* parent = parser.scopeParent();
		auto span = start.Span.merge(stream.LastToken().Span);
		auto field = make_unique<HXSLField>(span, parent, access, flags, name, move(symbol), semantic);

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case HXSLNodeType_Compilation:
			compilation->AddField(move(field));
			break;
		case HXSLNodeType_Namespace:
			parent->As<HXSLNamespace>()->AddField(move(field));
			break;
		case HXSLNodeType_Struct:
			parent->As<HXSLStruct>()->AddField(move(field));
			break;
		default:
			return false;
		}

		return true;
	}

	static bool ParseParameter(HXSLParser& parser, TokenStream& stream, HXSLNode* parent, unique_ptr<HXSLParameter>& parameter)
	{
		auto startingToken = stream.Current();

		HXSLParameterFlags flags = parser.ParseParameterFlags();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(HXSLSymbolRefType_AnyType, symbol));
		TextSpan name;
		IF_ERR_RET_FALSE(stream.ExpectIdentifier(name));

		TextSpan semantic = {};
		if (stream.TryGetOperator(HXSLOperator_Colon))
		{
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic));
		}

		auto span = startingToken.Span.merge(stream.LastToken().Span);

		parameter = make_unique<HXSLParameter>(span, parent, flags, move(symbol.make()), name, semantic);
		return true;
	}

	static bool ParseFunction(const Token& start, HXSLAccessModifier access, HXSLFunctionFlags flags, TextSpan name, unique_ptr<HXSLSymbolRef> returnSymbol, HXSLParser& parser, TokenStream& stream, HXSLCompilation* compilation, TakeHandle<HXSLAttributeDeclaration>* attribute)
	{
		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct)
		{
			return false;
		}

		auto parent = parser.scopeParent();
		auto function = make_unique<HXSLFunction>(TextSpan(), parent, access, flags, name, move(returnSymbol));
		if (attribute && attribute->HasResource())
		{
			function->AddAttribute(move(attribute->Take()));
		}

		vector<unique_ptr<HXSLParameter>> parameters;

		bool firstParameter = true;
		while (!stream.TryGetDelimiter(')'))
		{
			if (!firstParameter)
			{
				stream.ExpectDelimiter(',');
			}
			firstParameter = false;

			unique_ptr<HXSLParameter> parameter;

			IF_ERR_RET_FALSE(ParseParameter(parser, stream, function.get(), parameter));

			parameters.push_back(move(parameter));
		}

		function->SetParameters(move(parameters));

		if (stream.TryGetOperator(HXSLOperator_Colon))
		{
			TextSpan semantic;
			IF_ERR_RET_FALSE(stream.ExpectIdentifier(semantic));
			function->SetSemantic(semantic);
		}

		if (!stream.TryGetDelimiter(';'))
		{
			unique_ptr<HXSLBlockStatement> statement;
			IF_ERR_RET_FALSE(ParseStatementBody(name, ScopeType_Function, function.get(), parser, stream, statement));
			function->SetBody(move(statement));
		}

		function->SetSpan(stream.MakeFromLast(start));

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case HXSLNodeType_Compilation:
			compilation->AddFunction(move(function));
			break;
		case HXSLNodeType_Namespace:
			parent->As<HXSLNamespace>()->AddFunction(move(function));
			break;
		case HXSLNodeType_Struct:
			parent->As<HXSLStruct>()->AddFunction(move(function));
			break;
		default:
			return false;
		}

		return true;
	}

	bool HXSLDeclarationParser::TryParse(HXSLParser& parser, TokenStream& stream, HXSLCompilation* compilation)
	{
		auto startingToken = stream.Current();

		HXSLAccessModifier access = parser.ParseAccessModifier();
		HXSLFunctionFlags functionFlags = parser.ParseFunctionFlags();
		HXSLFieldFlags fieldFlags = parser.ParseFieldFlags();

		LazySymbol symbol;
		IF_ERR_RET_FALSE(parser.TryParseSymbol(HXSLSymbolRefType_AnyType, symbol));

		TextSpan name;
		IF_ERR_RET_FALSE(stream.TryGetIdentifier(name));

		TextSpan fieldSemantic;
		if (stream.TryGetDelimiter('('))
		{
			TakeHandle<HXSLAttributeDeclaration>* attribute = nullptr;
			parser.AcceptAttribute(&attribute, "");
			IF_ERR_RET_FALSE(ParseFunction(startingToken, access, functionFlags, name, move(symbol.make()), parser, stream, compilation, attribute));
			return true;
		}
		else if (stream.TryGetDelimiter(';'))
		{
			ParseField(startingToken, access, fieldFlags, name, move(symbol.make()), {}, parser, stream, compilation);
			return true;
		}
		else if (stream.TryGetOperator(HXSLOperator_Colon) && stream.TryGetIdentifier(fieldSemantic) && stream.TryGetDelimiter(';'))
		{
			ParseField(startingToken, access, fieldFlags, name, move(symbol.make()), fieldSemantic, parser, stream, compilation);
			return true;
		}

		return false;
	}

	bool HXSLStructParser::TryParse(HXSLParser& parser, TokenStream& stream, HXSLCompilation* compilation)
	{
		auto startingToken = stream.Current();

		auto access = parser.ParseAccessModifier();

		IF_ERR_RET_FALSE(stream.TryGetKeyword(HXSLKeyword_Struct));

		TextSpan name;
		IF_ERR_RET_FALSE(stream.ExpectIdentifier(name));

		IF_ERR_RET_FALSE(parser.AcceptAttribute(nullptr, "is not valid in this context on '%s'", name.toString().c_str()));

		auto scopeType = parser.scopeType();
		if (scopeType != ScopeType_Global && scopeType != ScopeType_Namespace && scopeType != ScopeType_Struct)
		{
			ERR_RETURN_FALSE(parser, "Cannot declare struct here.");
		}

		auto parent = parser.scopeParent();
		auto _struct = make_unique<HXSLStruct>(TextSpan(), parent, access, name);

		Token t;
		IF_ERR_RET_FALSE(parser.EnterScope(name, ScopeType_Struct, _struct.get(), t));
		while (parser.IterateScope())
		{
			while (HXSLSubParserRegistry::TryParse(parser, stream, _struct.get(), compilation));
		}

		stream.TryGetDelimiter(';'); // this is optional

		auto span = startingToken.Span.merge(stream.LastToken().Span);
		_struct->SetSpan(span);

		auto parentType = parent->GetType();
		switch (parentType)
		{
		case HXSLNodeType_Compilation:
			compilation->AddStruct(move(_struct));
			break;
		case HXSLNodeType_Namespace:
			parent->As<HXSLNamespace>()->AddStruct(move(_struct));
			break;
		case HXSLNodeType_Struct:
			parent->As<HXSLStruct>()->AddStruct(move(_struct));
			break;
		default:
			return false;
		}

		return true;
	}
}