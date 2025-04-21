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

	void HXSLParser::EnterScopeInternal(TextSpan name, ScopeType type, HXSLNode* userdata)
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

	void HXSLParser::ExitScopeInternal()
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

	bool HXSLParser::TryEnterScope(TextSpan name, ScopeType type, HXSLNode* parent)
	{
		if (Stream.TryGetDelimiter('{'))
		{
			EnterScopeInternal(name, type, parent);
			return true;
		}
		return false;
	}

	bool HXSLParser::EnterScope(TextSpan name, ScopeType type, HXSLNode* parent, Token& token)
	{
		ERR_IF_RETURN_FALSE(!Stream.ExpectDelimiter('{', token));
		EnterScopeInternal(name, type, parent);
		return true;
	}

	bool HXSLParser::EnterScope(TextSpan name, ScopeType type, HXSLNode* parent)
	{
		Token token;
		ERR_IF_RETURN_FALSE(!EnterScope(name, type, parent, token));
		return true;
	}

	bool HXSLParser::SkipScope(Token& token)
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

	bool HXSLParser::IterateScope()
	{
		if (Stream.TryGetDelimiter('}'))
		{
			ExitScopeInternal();
			return false;
		}
		return true;
	}

	UsingDeclaration HXSLParser::ParseUsingDeclaration()
	{
		auto nsKeywordSpan = Stream.LastToken().Span;
		UsingDeclaration us = UsingDeclaration();
		bool hasDot;
		TextSpan identifier = ParseQualifiedName(Stream, hasDot);
		if (hasDot)
		{
			us.Target = identifier;
		}
		else if (Stream.TryGetOperator(HXSLOperator_Equal))
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

	NamespaceDeclaration HXSLParser::ParseNamespaceDeclaration(bool& scoped)
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

	bool HXSLParser::TryAdvance()
	{
		if (Stream.Current().Type == TokenType_Unknown || Stream.HasErrors())
		{
			return false;
		}

		while (true)
		{
			if (Stream.TryGetKeyword(HXSLKeyword_Namespace))
			{
				if (ScopeLevel != 0) ERR_RETURN_FALSE_INTERNAL("Namespaces must be at the global scope.");
				if (CurrentNamespace != nullptr) ERR_RETURN_FALSE_INTERNAL("Only one namespace can be declared in the current scope.");
				bool scoped;
				CurrentNamespace = Compilation->AddNamespace(ParseNamespaceDeclaration(scoped));
				if (!scoped)
				{
					CurrentScope = ResolverScopeContext(CurrentNamespace->GetName(), ScopeType_Namespace, CurrentNamespace, ScopeFlags_None);
				}
				NamespaceScope = ScopeLevel;
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Using))
			{
				if (!IsInGlobalOrNamespaceScope()) ERR_RETURN_FALSE_INTERNAL("Usings must be at the global or namespace scope.");
				auto declaration = ParseUsingDeclaration();
				if (CurrentNamespace != nullptr)
				{
					CurrentNamespace->AddUsing(declaration);
				}
				else
				{
					Compilation->AddUsing(declaration);
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
			ERR_RETURN_FALSE_INTERNAL("Expected namespace.");
		}

		return true;
	}

	bool HXSLParser::Parse()
	{
		Stream.Advance();
		ParseSubStep();
		return !Stream.HasErrors();
	}

	bool HXSLParser::ParseSubStep()
	{
		while (TryAdvance())
		{
			std::unique_ptr<HXSLAttributeDeclaration> attr;
			TryParseAttribute(attr);
			if (attr)
			{
				attribute.Reset(std::move(attr));
			}
			HXSLSubParserRegistry::TryParse(*this, Stream, Compilation, Compilation);
		}
		return true;
	}

	HXSLAccessModifier HXSLParser::ParseAccessModifier()
	{
		HXSLAccessModifier modifier = HXSLAccessModifier_Private;
		while (true)
		{
			if (Stream.TryGetKeyword(HXSLKeyword_Public))
			{
				modifier = HXSLAccessModifier_Public;
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Internal))
			{
				modifier = HXSLAccessModifier_Internal;
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Private))
			{
				modifier = HXSLAccessModifier_Private;
			}
			else
			{
				break;
			}
		}

		return modifier;
	}

	static const std::unordered_set<HXSLKeyword> BuiltInTypes = {
		HXSLKeyword_Void,
		HXSLKeyword_Bool,
		HXSLKeyword_Uint,
		HXSLKeyword_Int,
		HXSLKeyword_Float,
		HXSLKeyword_Double,
		HXSLKeyword_Matrix,
		HXSLKeyword_Min16float,
		HXSLKeyword_Min10float,
		HXSLKeyword_Min16int,
		HXSLKeyword_Min12int,
		HXSLKeyword_Min16uint,
		HXSLKeyword_SamplerState,
		HXSLKeyword_Texture1D,
		HXSLKeyword_Texture2D,
		HXSLKeyword_Texture3D,
		HXSLKeyword_Texture1DArray,
		HXSLKeyword_Texture2DArray,
		HXSLKeyword_Texture2DMS,
		HXSLKeyword_Texture2DMSArray,
		HXSLKeyword_TextureCube
	};

	bool HXSLParser::TryParseSymbol(HXSLSymbolRefType expectedType, LazySymbol& type)
	{
		TextSpan span;
		if (expectedType != HXSLSymbolRefType_Variable && expectedType != HXSLSymbolRefType_Attribute && Stream.TryGetKeywords(BuiltInTypes))
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

	bool HXSLParser::ParseSymbol(HXSLSymbolRefType expectedType, std::unique_ptr<HXSLSymbolRef>& type)
	{
		LazySymbol symbol;
		if (!TryParseSymbol(expectedType, symbol))
		{
			ERR_RETURN_FALSE_INTERNAL("Expected an symbol.");
		}

		type = symbol.make();

		return true;
	}

	HXSLParameterFlags HXSLParser::ParseParameterFlags()
	{
		HXSLParameterFlags flags = HXSLParameterFlags_None;
		while (true)
		{
			if (Stream.TryGetKeyword(HXSLKeyword_In))
			{
				flags = static_cast<HXSLParameterFlags>(flags | HXSLParameterFlags_In);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Out))
			{
				flags = static_cast<HXSLParameterFlags>(flags | HXSLParameterFlags_Out);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Inout))
			{
				flags = static_cast<HXSLParameterFlags>(flags | HXSLParameterFlags_InOut);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Uniform))
			{
				flags = static_cast<HXSLParameterFlags>(flags | HXSLParameterFlags_Uniform);
			}
			else
			{
				break;
			}
		}

		return flags;
	}

	HXSLFunctionFlags HXSLParser::ParseFunctionFlags()
	{
		HXSLFunctionFlags flags = HXSLFunctionFlags_None;
		while (true)
		{
			if (Stream.TryGetKeyword(HXSLKeyword_Inline))
			{
				flags = static_cast<HXSLFunctionFlags>(flags | HXSLFunctionFlags_Inline);
			}
			else
			{
				break;
			}
		}

		return flags;
	}

	HXSLFieldFlags HXSLParser::ParseFieldFlags()
	{
		HXSLFieldFlags flags = HXSLFieldFlags_None;
		while (true)
		{
			if (Stream.TryGetKeyword(HXSLKeyword_Static))
			{
				flags = static_cast<HXSLFieldFlags>(flags | HXSLFieldFlags_Static);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Nointerpolation))
			{
				flags = static_cast<HXSLFieldFlags>(flags | HXSLFieldFlags_Nointerpolation);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Shared))
			{
				flags = static_cast<HXSLFieldFlags>(flags | HXSLFieldFlags_Shared);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Groupshared))
			{
				flags = static_cast<HXSLFieldFlags>(flags | HXSLFieldFlags_GroupShared);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Uniform))
			{
				flags = static_cast<HXSLFieldFlags>(flags | HXSLFieldFlags_Uniform);
			}
			else if (Stream.TryGetKeyword(HXSLKeyword_Volatile))
			{
				flags = static_cast<HXSLFieldFlags>(flags | HXSLFieldFlags_Volatile);
			}
			else
			{
				break;
			}
		}

		return flags;
	}

	bool HXSLParser::TryParseAttribute(std::unique_ptr<HXSLAttributeDeclaration>& attributeOut)
	{
		auto start = Stream.Current();
		IF_ERR_RET_FALSE(Stream.TryGetDelimiter('['));

		std::unique_ptr<HXSLSymbolRef> symbol;
		IF_ERR_RET_FALSE(ParseSymbol(HXSLSymbolRefType_Attribute, symbol));

		auto attribute = std::make_unique<HXSLAttributeDeclaration>(TextSpan(), nullptr);

		std::vector<std::unique_ptr<HXSLLiteralExpression>> parameters;
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
				std::unique_ptr<HXSLLiteralExpression> parameter;
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