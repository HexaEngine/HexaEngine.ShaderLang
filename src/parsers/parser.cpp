#include "parser.h"
#include "sub_parser_registry.hpp"
#include "parsers/parser_helper.hpp"
#include "pratt_parser.hpp"
namespace HXSL
{
#define ERR_RETURN_FALSE_INTERNAL(message) \
	do { \
		LogError(message, stream->Current()); \
		return false; \
	} while (0)

	void Parser::InitializeSubSystems()
	{
		SubParserRegistry::EnsureCreated();
		StatementParserRegistry::EnsureCreated();
		ExpressionParserRegistry::EnsureCreated();
	}

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
		case ScopeType_Class:
			newFlags |= ScopeFlags_InsideClass;
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
		CurrentScope = ParserScopeContext(name, type, userdata, newFlags);
		ScopeLevel++;
	}

	void Parser::ExitScopeInternal()
	{
		ScopeLevel--;
		if (ScopeLevel < 0)
		{
			//HXSL_ASSERT(false, "Scope level cannot be smaller than 0.");
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
		if (stream->TryGetDelimiter('{'))
		{
			EnterScopeInternal(name, type, parent);
			return true;
		}
		return false;
	}

	bool Parser::EnterScope(TextSpan name, ScopeType type, ASTNode* parent, Token& token, bool pretendOnError)
	{
		if (!stream->ExpectDelimiter('{', token) && !pretendOnError)
		{
			return false;
		}
		EnterScopeInternal(name, type, parent);
		return true;
	}

	void Parser::RestoreFromPoint()
	{
		stream->RestoreFromPoint();
		modifierList = {};
		attribute.Reset();
	}

	bool Parser::EnterScope(TextSpan name, ScopeType type, ASTNode* parent, bool pretendOnError)
	{
		Token token;
		ERR_IF_RETURN_FALSE(!EnterScope(name, type, parent, token, pretendOnError));
		return true;
	}

	bool Parser::SkipScope(Token& token)
	{
		int targetScope = ScopeLevel - 1;
		while (TryAdvance())
		{
			if (ScopeLevel == targetScope)
			{
				token = stream->LastToken();
				return true;
			}
		}

		if (ScopeLevel == targetScope)
		{
			token = stream->LastToken();
			return true;
		}

		LogError("Unexpected end of tokens.", stream->Current());
		return false;
	}

	static const std::unordered_set<Keyword> scopeRecoveryPoints =
	{
		Keyword_Namespace,
		Keyword_Class,
		Keyword_Struct,
		Keyword_Explicit,
		Keyword_Implicit,
		Keyword_Operator,
		Keyword_Export,
		Keyword_Extern,
		Keyword_Private,
		Keyword_Internal,
		Keyword_Protected,
		Keyword_Public,
		Keyword_Interface,
		Keyword_Static,
		Keyword_Volatile,
		Keyword_Typedef,
		Keyword_NoInterpolation,
		Keyword_Noperspective,
		Keyword_Centroid,
		Keyword_Linear,
		Keyword_Sample,
		Keyword_Uniform,
		Keyword_In,
		Keyword_Out,
		Keyword_Inout,
		Keyword_Shared,
		Keyword_Groupshared,
	};

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

	static const std::unordered_set<Keyword> flowControlKeywords =
	{
		Keyword_Return,
		Keyword_Break,
		Keyword_Continue,
		Keyword_Discard,
		Keyword_If,
		Keyword_Else,
		Keyword_While,
		Keyword_Do,
		Keyword_For,
		Keyword_Switch,
		Keyword_Case,
		Keyword_Default,
	};

	bool Parser::IterateScope()
	{
		if (stream->TryGetDelimiter('}'))
		{
			ExitScopeInternal();
			return false;
		}

		if (stream->IsEndOfTokens())
		{
			LogError("Unexpected token, expected an '}'.", stream->LastToken());
		}

		return true;
	}

	bool Parser::TryRecoverScope(bool exitScope)
	{
		RestoreFromPoint();
		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			auto current = stream->Current();
			if (current.isKeywordOf(scopeRecoveryPoints))
			{
				LogError("Unexpected token, expected an '}'.", current);
				if (exitScope)
				{
					ExitScopeInternal();
				}
				return false;
			}
			if (current.isKeywordOf(BuiltInTypes) || current.isKeywordOf(flowControlKeywords) || current.isIdentifier())
			{
				return true;
			}

			stream->Advance();
		}
		return false;
	}

	static const std::unordered_set<Keyword> interpolationModifierKeywords =
	{
		Keyword_Linear,
		Keyword_Centroid,
		Keyword_NoInterpolation,
		Keyword_Noperspective,
		Keyword_Sample
	};

	static const std::unordered_set<Keyword> parameterFlagKeywords =
	{
		Keyword_In,
		Keyword_Out,
		Keyword_Inout,
		Keyword_Uniform,
		Keyword_Precise
	};

	static const std::unordered_set<char> parameterListExitRecoveryPoints =
	{
		'{', '}', ';', ')', '.', '$'
	};

	bool Parser::TryRecoverParameterList()
	{
		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			auto current = stream->Current();
			if (current.isIdentifier() || current.isKeywordOf(parameterFlagKeywords) || current.isKeywordOf(interpolationModifierKeywords))
			{
				return true;
			}
			if (current.isKeyword() || current.isOperator() || current.isDelimiterOf(parameterListExitRecoveryPoints))
			{
				return false;
			}
			stream->Advance();
			if (stream->Current().isDelimiterOf(','))
			{
				return true;
			}
		}

		return false;
	}

	void Parser::TryRecoverStatement()
	{
		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			auto current = stream->Current();
			if (current.isKeyword() || current.isDelimiterOf(';'))
			{
				return;
			}
			stream->Advance();
		}
	}

	UsingDeclaration Parser::ParseUsingDeclaration()
	{
		auto nsKeywordSpan = stream->LastToken().Span;
		UsingDeclaration us = UsingDeclaration();
		bool hasDot;
		TextSpan identifier = ParseQualifiedName(*stream, hasDot);
		if (hasDot)
		{
			us.Target = identifier;
		}
		else if (stream->TryGetOperator(Operator_Equal))
		{
			us.Alias = identifier;
			us.Target = ParseQualifiedName(*stream, hasDot);
			us.IsAlias = true;
		}
		else
		{
			us.Target = identifier;
		}
		stream->ExpectDelimiter(';');
		us.Span = nsKeywordSpan.merge(stream->LastToken().Span);
		return us;
	}

	NamespaceDeclaration Parser::ParseNamespaceDeclaration(bool& scoped)
	{
		auto nsKeywordSpan = stream->LastToken().Span;
		bool hasDot;
		auto name = ParseQualifiedName(*stream, hasDot);
		if (TryEnterScope(name, ScopeType_Namespace, nullptr))
		{
			scoped = true;
		}
		else
		{
			scoped = false;
			stream->ExpectDelimiter(';', "Expected a semicolon after namespace declaration");
		}

		return NamespaceDeclaration(nsKeywordSpan.merge(stream->LastToken().Span), name);
	}

	bool Parser::TryAdvance()
	{
		if (stream->Current().Type == TokenType_Unknown || stream->HasCriticalErrors())
		{
			return false;
		}

		while (true)
		{
			if (stream->TryGetKeyword(Keyword_Namespace))
			{
				if (ScopeLevel != 0) ERR_RETURN_FALSE_INTERNAL("Namespaces must be at the global scope.");
				if (CurrentNamespace != nullptr) ERR_RETURN_FALSE_INTERNAL("Only one namespace HXSL can be declared in the current scope.");
				bool scoped;
				CurrentNamespace = m_compilation->AddNamespace(ParseNamespaceDeclaration(scoped));
				if (!scoped)
				{
					CurrentScope = ParserScopeContext(CurrentNamespace->GetName(), ScopeType_Namespace, CurrentNamespace, ScopeFlags_None);
				}
				NamespaceScope = ScopeLevel;
			}
			else if (stream->TryGetKeyword(Keyword_Using))
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
			else if (stream->TryGetDelimiter('{'))
			{
				EnterScopeInternal({ }, ScopeType_Unknown, nullptr);
				return false;
			}
			else if (stream->TryGetDelimiter('}'))
			{
				ExitScopeInternal();
				return false;
			}
			else
			{
				break;
			}

			if (stream->IsEndOfTokens())
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
		stream->Advance();
		while (TryAdvance())
		{
			if (!ParseSubStepInner(m_compilation))
			{
				LogError("Unexpected token", stream->Current());
				stream->TryAdvance();
			}
		}
		return !stream->HasCriticalErrors();
	}

	bool Parser::ParseSubStep(ASTNode* parent)
	{
		while (ParseSubStepInner(parent))
		{
		}

		return true;
	}

	void Parser::ParseInnerBegin()
	{
		stream->MakeRestorePoint();
		ParseAttribute();
		ParseModifierList();
	}

	bool Parser::ParseSubStepInner(ASTNode* parent)
	{
		ParseInnerBegin();
		if (SubParserRegistry::TryParse(*this, *stream, parent, m_compilation))
		{
			HXSL_ASSERT(modifierList.Empty(), "Modifier list was not empty, forgot to accept/reject it?.");
			HXSL_ASSERT(!attribute.HasResource(), "Attribute list was not empty, forgot to accept/reject it?.");
			return true;
		}
		else
		{
			return false;
		}
	}

	static bool IsRecoveryToken(const Token& token)
	{
		if (token.isDelimiterOf(';') || token.isDelimiterOf('}')) return true;
		if (token.isKeyword()) return true;
		if (token.isIdentifier()) return true;
		return false;
	}

	bool Parser::AttemptErrorRecovery(bool restorePoint)
	{
		if (restorePoint)
		{
			RestoreFromPoint();
		}

		stream->Advance();
		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			Token token = stream->Current();

			if (IsRecoveryToken(token))
			{
				return true;
			}

			stream->Advance();
		}
		return false;
	}

	bool Parser::TryParseSymbolInternal(const SymbolRefType& type, TextSpan& span)
	{
		if (type != SymbolRefType_Attribute && stream->TryGetKeywords(BuiltInTypes))
		{
			span = stream->LastToken().Span;
			return true;
		}
		else if (stream->TryGetIdentifier(span))
		{
			return true;
		}

		return false;
	}

	void Parser::ParseModifierList()
	{
		ModifierList list = {};
		auto start = stream->Current();
		list.accessModifiers = ParseAccessModifiers(list.anyAccessModifiersSpecified);
		list.functionFlags = ParseFunctionFlags();
		list.storageClasses = ParseStorageClasses();
		list.interpolationModifiers = ParseInterpolationModifiers(list.anyInterpolationModifiersSpecified);
		list.span = stream->MakeFromLast(start);
		modifierList = list;
	}

	AccessModifier Parser::ParseAccessModifiers(bool& anySpecified)
	{
		AccessModifier modifier = AccessModifier_None;

		bool hasPublic = false;
		bool hasPrivate = false;
		bool hasProtected = false;
		bool hasInternal = false;

		bool hadError = false;

		while (true)
		{
			auto start = stream->Current();
			if (stream->TryGetKeyword(Keyword_Public))
			{
				if (hasPrivate || hasProtected || hasInternal)
				{
					LogError("Invalid combination: 'public' cannot be combined with 'private', 'protected', or 'internal'.", start.Span);
					hadError = true;
				}
				modifier |= AccessModifier_Public;
				hasPublic = true;
			}
			else if (stream->TryGetKeyword(Keyword_Internal))
			{
				if (hasPrivate || hasPublic)
				{
					LogError("Invalid combination: 'internal' cannot be combined with 'private' or 'public'.", start.Span);
					hadError = true;
				}
				modifier |= AccessModifier_Internal;
				hasInternal = true;
			}
			else if (stream->TryGetKeyword(Keyword_Protected))
			{
				if (hasPrivate || hasPublic)
				{
					LogError("Invalid combination: 'protected' cannot be combined with 'private' or 'public'.", start.Span);
					hadError = true;
				}
				modifier |= AccessModifier_Protected;
				hasProtected = true;
			}
			else if (stream->TryGetKeyword(Keyword_Private))
			{
				if (hasPublic || hasProtected || hasInternal)
				{
					LogError("Invalid combination: 'private' cannot be combined with 'public', 'protected', or 'internal'.", start.Span);
					hadError = true;
				}
				modifier |= AccessModifier_Private;
				hasPrivate = true;
			}
			else
			{
				break;
			}
		}

		anySpecified = modifier != AccessModifier_None;

		if (!anySpecified)
		{
			return AccessModifier_Private;
		}

		if (hadError)
		{
			return AccessModifier_Private;
		}

		return modifier;
	}

	std::tuple<ParameterFlags, InterpolationModifier> Parser::ParseParameterFlags()
	{
		ParameterFlags flags = ParameterFlags_In;

		bool hasIn = false;
		bool hasOut = false;
		bool hasInOut = false;

		bool hadError = false;

		while (true)
		{
			auto start = stream->Current();

			if (stream->TryGetKeyword(Keyword_In))
			{
				if (hasOut || hasInOut)
				{
					LogError("Invalid combination: 'in' cannot be combined with 'out' or 'inout'.", start.Span);
					hadError = true;
				}
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_In);
				hasIn = true;
			}
			else if (stream->TryGetKeyword(Keyword_Out))
			{
				if (hasIn || hasInOut)
				{
					LogError("Invalid combination: 'out' cannot be combined with 'in' or 'inout'.", start.Span);
					hadError = true;
				}
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_Out);
				hasOut = true;
			}
			else if (stream->TryGetKeyword(Keyword_Inout))
			{
				if (hasIn || hasOut)
				{
					LogError("Invalid combination: 'inout' cannot be combined with 'in' or 'out'.", start.Span);
					hadError = true;
				}
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_InOut);
				hasInOut = true;
			}
			else if (stream->TryGetKeyword(Keyword_Uniform))
			{
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_Uniform);
			}
			else if (stream->TryGetKeyword(Keyword_Precise))
			{
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_Precise);
			}
			else
			{
				break;
			}
		}

		bool dummy;
		auto interpolationMods = ParseInterpolationModifiers(dummy);

		return std::make_tuple(flags, interpolationMods);
	}

	InterpolationModifier Parser::ParseInterpolationModifiers(bool& anySpecified)
	{
		InterpolationModifier modifier = InterpolationModifier_None;

		bool hasLinear = false;
		bool hasCentroid = false;
		bool hasNoInterpolation = false;
		bool hasNoPerspective = false;
		bool hasSample = false;

		bool hadError = false;

		while (true)
		{
			auto start = stream->Current();

			if (stream->TryGetKeyword(Keyword_Linear))
			{
				if (hasCentroid || hasNoInterpolation)
				{
					LogError("Invalid combination: 'linear' cannot be combined with 'centroid' or 'nointerpolation'.", start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_Linear;
				hasLinear = true;
			}
			else if (stream->TryGetKeyword(Keyword_Centroid))
			{
				if (hasLinear || hasNoInterpolation)
				{
					LogError("Invalid combination: 'centroid' cannot be combined with 'linear' or 'nointerpolation'.", start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_Centroid;
				hasCentroid = true;
			}
			else if (stream->TryGetKeyword(Keyword_NoInterpolation))
			{
				if (hasLinear || hasCentroid || hasSample || hasNoPerspective)
				{
					LogError("Invalid combination: 'nointerpolation' cannot be combined with other interpolation modifiers.", start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_NoInterpolation;
				hasNoInterpolation = true;
			}
			else if (stream->TryGetKeyword(Keyword_Noperspective))
			{
				if (hasNoInterpolation)
				{
					LogError("Invalid combination: 'noperspecitve' cannot be combined with 'nointerpolation'.", start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_NoPerspecitve;
				hasNoPerspective = true;
			}
			else if (stream->TryGetKeyword(Keyword_Sample))
			{
				if (hasNoInterpolation)
				{
					LogError("Invalid combination: 'sample' cannot be combined with 'nointerpolation'.", start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_Sample;
				hasSample = true;
			}
			else
			{
				break;
			}
		}

		anySpecified = modifier != InterpolationModifier_None;

		if ((modifier & ~(InterpolationModifier_Linear | InterpolationModifier_Centroid | InterpolationModifier_NoInterpolation)) == 0)
		{
			modifier |= InterpolationModifier_Linear;
		}

		if (hadError)
		{
			return InterpolationModifier_Linear;
		}

		return modifier;
	}

	FunctionFlags Parser::ParseFunctionFlags()
	{
		FunctionFlags flags = FunctionFlags_None;
		while (true)
		{
			if (stream->TryGetKeyword(Keyword_Inline))
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

	StorageClass Parser::ParseStorageClasses()
	{
		StorageClass flags = StorageClass_None;
		while (true)
		{
			if (stream->TryGetKeyword(Keyword_Const))
			{
				flags = static_cast<StorageClass>(flags | StorageClass_Const);
			}
			else if (stream->TryGetKeyword(Keyword_Static))
			{
				flags = static_cast<StorageClass>(flags | StorageClass_Static);
			}
			else if (stream->TryGetKeyword(Keyword_Precise))
			{
				flags = static_cast<StorageClass>(flags | StorageClass_Precise);
			}
			else if (stream->TryGetKeyword(Keyword_Shared))
			{
				flags = static_cast<StorageClass>(flags | StorageClass_Shared);
			}
			else if (stream->TryGetKeyword(Keyword_Groupshared))
			{
				flags = static_cast<StorageClass>(flags | StorageClass_GroupShared);
			}
			else if (stream->TryGetKeyword(Keyword_Uniform))
			{
				flags = static_cast<StorageClass>(flags | StorageClass_Uniform);
			}
			else if (stream->TryGetKeyword(Keyword_Volatile))
			{
				flags = static_cast<StorageClass>(flags | StorageClass_Volatile);
			}
			else
			{
				break;
			}
		}

		return flags;
	}

	void Parser::ParseAttribute()
	{
		auto start = stream->Current();
		IF_ERR_RET(stream->TryGetDelimiter('['));

		std::unique_ptr<SymbolRef> symbol;
		IF_ERR_RET(ParseSymbol(SymbolRefType_Attribute, symbol));

		auto attribute = std::make_unique<AttributeDeclaration>(TextSpan(), nullptr);

		std::vector<std::unique_ptr<Expression>> parameters;
		if (stream->TryGetDelimiter('('))
		{
			bool firstParam = true;
			while (!stream->TryGetDelimiter(')'))
			{
				if (!firstParam)
				{
					IF_ERR_RET(stream->ExpectDelimiter(','));
				}
				firstParam = false;
				std::unique_ptr<Expression> parameter;
				if (!PrattParser::ParseExpression(*this, *stream, attribute.get(), parameter))
				{
					return;
				}
				parameters.push_back(std::move(parameter));
			}
		}

		IF_ERR_RET(stream->ExpectDelimiter(']'));
		attribute->SetParameters(std::move(parameters));
		attribute->SetSymbol(std::move(symbol));
		attribute->SetSpan(stream->MakeFromLast(start));
		this->attribute.Reset(std::move(attribute));
	}
}