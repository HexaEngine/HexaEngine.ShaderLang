#include "parser.h"
#include "sub_parser_registry.hpp"
#include "parsers/parser_helper.hpp"
namespace HXSL
{
#define ERR_RETURN_FALSE_INTERNAL(message) \
	do { \
		LogError(message); \
		return false; \
	} while (0)

	void Parser::EnterScopeInternal(TextSpan name, ScopeType type, ASTNode* userdata)
	{
		ScopeFlags newFlags = CurrentScope.Flags;
		switch (type) {
		case ScopeType_Global:
			newFlags |= ScopeFlags_InsideGlobal;
			break;
		case ScopeType_Namespace:
			newFlags |= ScopeFlags_InsideNamespace;
			break;
		case ScopeType_Struct:
			newFlags |= ScopeFlags_InsideStruct;
			break;
		case ScopeType_Function:
			newFlags |= ScopeFlags_InsideFunction;
			break;
		case ScopeType_If:
			newFlags |= ScopeFlags_InsideIf;
			break;
		case ScopeType_Else:
			newFlags |= ScopeFlags_InsideElse;
			break;
		case ScopeType_ElseIf:
			newFlags |= ScopeFlags_InsideElseIf;
			break;
		case ScopeType_While:
			newFlags |= ScopeFlags_InsideWhile;
			break;
		case ScopeType_For:
			newFlags |= ScopeFlags_InsideFor;
			break;
		case ScopeType_Switch:
			newFlags |= ScopeFlags_InsideSwitch;
			break;
		case ScopeType_Initialization:
			newFlags |= ScopeFlags_InsideInitialization;
			break;
		default:
			break;
		}

		ScopeStack.push(CurrentScope);
		CurrentScope = ResolverScopeContext(name, type, userdata, newFlags);
		ScopeLevel++;
	}

	void Parser::ExitScopeInternal()
	{
		ScopeLevel--;
		if (ScopeLevel < 0)
		{
			HXSL_ASSERT(false, "Scope level cannot be smaller than 0.");
			return;
		}

		CurrentScope = ScopeStack.top();
		ScopeStack.pop();

		if (ScopeLevel < NamespaceScope)
		{
			NamespaceScope = 0;
			CurrentNamespace = nullptr;
		}
	}

	bool Parser::TryEnterScope(TextSpan name, ScopeType type, ASTNode* parent)
	{
		if (Stream.TryGetDelimiter('{'))
		{
			EnterScopeInternal(name, type, parent);
			return true;
		}
		return false;
	}

	bool Parser::EnterScope(TextSpan name, ScopeType type, ASTNode* parent, Token& token)
	{
		ERR_IF_RETURN_FALSE(!Stream.ExpectDelimiter('{', token));
		EnterScopeInternal(name, type, parent);
		return true;
	}

	bool Parser::EnterScope(TextSpan name, ScopeType type, ASTNode* parent)
	{
		Token token;
		ERR_IF_RETURN_FALSE(!EnterScope(name, type, parent, token));
		return true;
	}

	bool Parser::SkipScope(Token& token)
	{
		int targetScope = ScopeLevel - 1;
		while (TryAdvance())
		{
			if (ScopeLevel == targetScope)
			{
				token = Stream.LastToken();
				return true;
			}
		}

		if (ScopeLevel == targetScope)
		{
			token = Stream.LastToken();
			return true;
		}

		LogError("Unexpected end of tokens.");
		return false;
	}

	bool Parser::IterateScope()
	{
		if (Stream.TryGetDelimiter('}'))
		{
			ExitScopeInternal();
			return false;
		}
		return true;
	}

	UsingDeclaration Parser::ParseUsingDeclaration()
	{
		auto nsKeywordSpan = Stream.LastToken().Span;
		UsingDeclaration us = UsingDeclaration();
		bool hasDot;
		TextSpan identifier = ParseQualifiedName(Stream, hasDot);
		if (hasDot)
		{
			us.Target = identifier;
		}
		else if (Stream.TryGetOperator(Operator_Equal))
		{
			us.Alias = identifier;
			us.Target = ParseQualifiedName(Stream, hasDot);
			us.IsAlias = true;
		}
		else
		{
			us.Target = identifier;
		}
		Stream.ExpectDelimiter(';');
		us.Span = nsKeywordSpan.merge(Stream.LastToken().Span);
		return us;
	}

	NamespaceDeclaration Parser::ParseNamespaceDeclaration(bool& scoped)
	{
		auto nsKeywordSpan = Stream.LastToken().Span;
		bool hasDot;
		auto name = ParseQualifiedName(Stream, hasDot);
		if (TryEnterScope(name, ScopeType_Namespace, nullptr))
		{
			scoped = true;
		}
		else
		{
			scoped = false;
			if (!Stream.ExpectDelimiter(';'))
			{
				return { };
			}
		}

		return NamespaceDeclaration(nsKeywordSpan.merge(Stream.LastToken().Span), name);
	}

	bool Parser::TryAdvance()
	{
		if (Stream.Current().Type == TokenType_Unknown || Stream.HasErrors())
		{
			return false;
		}

		while (true)
		{
			if (Stream.TryGetKeyword(Keyword_Namespace))
			{
				if (ScopeLevel != 0) ERR_RETURN_FALSE_INTERNAL("Namespaces must be at the global scope.");
				if (CurrentNamespace != nullptr) ERR_RETURN_FALSE_INTERNAL("Only one namespace HXSL can be declared in the current scope.");
				bool scoped;
				CurrentNamespace = m_compilation->AddNamespace(ParseNamespaceDeclaration(scoped));
				if (!scoped)
				{
					CurrentScope = ResolverScopeContext(CurrentNamespace->GetName(), ScopeType_Namespace, CurrentNamespace, ScopeFlags_None);
				}
				NamespaceScope = ScopeLevel;
			}
			else if (Stream.TryGetKeyword(Keyword_Using))
			{
				if (!IsInGlobalOrNamespaceScope()) ERR_RETURN_FALSE_INTERNAL("Usings must be at the global or namespace HXSL scope.");
				auto declaration = ParseUsingDeclaration();
				if (CurrentNamespace != nullptr)
				{
					CurrentNamespace->AddUsing(declaration);
				}
				else
				{
					m_compilation->AddUsing(declaration);
				}
			}
			else if (Stream.TryGetDelimiter('{'))
			{
				EnterScopeInternal({ }, ScopeType_Unknown, nullptr);
				return false;
			}
			else if (Stream.TryGetDelimiter('}'))
			{
				ExitScopeInternal();
				return false;
			}
			else
			{
				break;
			}

			if (Stream.IsEndOfTokens())
			{
				return false;
			}
		}

		if (CurrentNamespace == nullptr)
		{
			ERR_RETURN_FALSE_INTERNAL("Expected namespace HXSL.");
		}

		return true;
	}

	bool Parser::Parse()
	{
		Stream.Advance();
		ParseSubStep();
		return !Stream.HasErrors();
	}

	bool Parser::ParseSubStep()
	{
		while (TryAdvance())
		{
			std::unique_ptr<AttributeDeclaration> attr;
			TryParseAttribute(attr);
			if (attr)
			{
				attribute.Reset(std::move(attr));
			}
			SubParserRegistry::TryParse(*this, Stream, m_compilation, m_compilation);
		}
		return true;
	}

	AccessModifier Parser::ParseAccessModifier()
	{
		AccessModifier modifier = AccessModifier_Private;
		while (true)
		{
			if (Stream.TryGetKeyword(Keyword_Public))
			{
				modifier = AccessModifier_Public;
			}
			else if (Stream.TryGetKeyword(Keyword_Internal))
			{
				modifier = AccessModifier_Internal;
			}
			else if (Stream.TryGetKeyword(Keyword_Private))
			{
				modifier = AccessModifier_Private;
			}
			else
			{
				break;
			}
		}

		return modifier;
	}

	static const std::unordered_set<Keyword> BuiltInTypes = {
		Keyword_Void,
		Keyword_Bool,
		Keyword_Uint,
		Keyword_Int,
		Keyword_Float,
		Keyword_Double,
		Keyword_Matrix,
		Keyword_Min16float,
		Keyword_Min10float,
		Keyword_Min16int,
		Keyword_Min12int,
		Keyword_Min16uint,
		Keyword_SamplerState,
		Keyword_Texture1D,
		Keyword_Texture2D,
		Keyword_Texture3D,
		Keyword_Texture1DArray,
		Keyword_Texture2DArray,
		Keyword_Texture2DMS,
		Keyword_Texture2DMSArray,
		Keyword_TextureCube
	};

	bool Parser::TryParseSymbol(SymbolRefType expectedType, LazySymbol& type)
	{
		TextSpan span;
		if (expectedType != SymbolRefType_Variable && expectedType != SymbolRefType_Attribute && Stream.TryGetKeywords(BuiltInTypes))
		{
			type = LazySymbol(Stream.LastToken(), expectedType);
			return true;
		}
		else if (Stream.TryGetIdentifier(span))
		{
			type = LazySymbol(Stream.LastToken(), expectedType);
			return true;
		}

		return false;
	}

	bool Parser::ParseSymbol(SymbolRefType expectedType, std::unique_ptr<SymbolRef>& type)
	{
		LazySymbol symbol;
		if (!TryParseSymbol(expectedType, symbol))
		{
			ERR_RETURN_FALSE_INTERNAL("Expected an symbol.");
		}

		type = symbol.make();

		return true;
	}

	ParameterFlags Parser::ParseParameterFlags()
	{
		ParameterFlags flags = ParameterFlags_None;
		while (true)
		{
			if (Stream.TryGetKeyword(Keyword_In))
			{
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_In);
			}
			else if (Stream.TryGetKeyword(Keyword_Out))
			{
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_Out);
			}
			else if (Stream.TryGetKeyword(Keyword_Inout))
			{
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_InOut);
			}
			else if (Stream.TryGetKeyword(Keyword_Uniform))
			{
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_Uniform);
			}
			else
			{
				break;
			}
		}

		return flags;
	}

	FunctionFlags Parser::ParseFunctionFlags()
	{
		FunctionFlags flags = FunctionFlags_None;
		while (true)
		{
			if (Stream.TryGetKeyword(Keyword_Inline))
			{
				flags = static_cast<FunctionFlags>(flags | FunctionFlags_Inline);
			}
			else
			{
				break;
			}
		}

		return flags;
	}

	FieldFlags Parser::ParseFieldFlags()
	{
		FieldFlags flags = FieldFlags_None;
		while (true)
		{
			if (Stream.TryGetKeyword(Keyword_Static))
			{
				flags = static_cast<FieldFlags>(flags | FieldFlags_Static);
			}
			else if (Stream.TryGetKeyword(Keyword_Nointerpolation))
			{
				flags = static_cast<FieldFlags>(flags | FieldFlags_Nointerpolation);
			}
			else if (Stream.TryGetKeyword(Keyword_Shared))
			{
				flags = static_cast<FieldFlags>(flags | FieldFlags_Shared);
			}
			else if (Stream.TryGetKeyword(Keyword_Groupshared))
			{
				flags = static_cast<FieldFlags>(flags | FieldFlags_GroupShared);
			}
			else if (Stream.TryGetKeyword(Keyword_Uniform))
			{
				flags = static_cast<FieldFlags>(flags | FieldFlags_Uniform);
			}
			else if (Stream.TryGetKeyword(Keyword_Volatile))
			{
				flags = static_cast<FieldFlags>(flags | FieldFlags_Volatile);
			}
			else
			{
				break;
			}
		}

		return flags;
	}

	bool Parser::TryParseAttribute(std::unique_ptr<AttributeDeclaration>& attributeOut)
	{
		auto start = Stream.Current();
		IF_ERR_RET_FALSE(Stream.TryGetDelimiter('['));

		std::unique_ptr<SymbolRef> symbol;
		IF_ERR_RET_FALSE(ParseSymbol(SymbolRefType_Attribute, symbol));

		auto attribute = std::make_unique<AttributeDeclaration>(TextSpan(), nullptr);

		std::vector<std::unique_ptr<LiteralExpression>> parameters;
		if (Stream.TryGetDelimiter('('))
		{
			bool firstParam = true;
			while (!Stream.TryGetDelimiter(')'))
			{
				if (!firstParam)
				{
					IF_ERR_RET_FALSE(Stream.ExpectDelimiter(','));
				}
				firstParam = false;
				std::unique_ptr<LiteralExpression> parameter;
				if (!ParserHelper::TryParseLiteralExpression(*this, Stream, attribute.get(), parameter))
				{
					LogError("Expected an constant expression.");
					return false;
				}
				parameters.push_back(std::move(parameter));
			}
		}

		IF_ERR_RET_FALSE(Stream.ExpectDelimiter(']'));
		attribute->SetParameters(std::move(parameters));
		attribute->SetSymbol(std::move(symbol));
		attribute->SetSpan(Stream.MakeFromLast(start));
		attributeOut = std::move(attribute);
		return true;
	}
}