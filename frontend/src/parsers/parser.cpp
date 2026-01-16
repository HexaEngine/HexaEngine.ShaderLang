#include "parser.hpp"
#include "sub_parser_registry.hpp"
#include "parsers/parser_helper.hpp"
#include "hybrid_expr_parser.hpp"
#include "declaration_parser.hpp"
namespace HXSL
{
#define ERR_RETURN_FALSE_INTERNAL(code) \
	do { \
		Log(code, stream->Current()); \
		return false; \
	} while (0)

	CompilationUnit* CompilationUnitBuilder::Build(bool isExtern)
	{
		return CompilationUnit::Create(isExtern, builder.namespaces, builder.usings);
	}

	void Parser::InitializeSubSystems()
	{
		SubParserRegistry::EnsureCreated();
		StatementParserRegistry::EnsureCreated();
		ExpressionParserRegistry::EnsureCreated();
	}

	void Parser::EnterScopeInternal(ScopeType type, ASTNode* userdata)
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
		CurrentScope = ParserScopeContext(type, userdata, newFlags);
		ScopeLevel++;
	}

	void Parser::ExitScopeInternal(ASTNode* parent)
	{
		if (CurrentScope.Parent != parent) return;
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
		}
	}

	bool Parser::TryEnterScope(ScopeType type, ASTNode* parent)
	{
		if (stream->TryGetDelimiter('{'))
		{
			EnterScopeInternal(type, parent);
			return true;
		}
		return false;
	}

	bool Parser::EnterScope(ScopeType type, ASTNode* parent, Token& token, bool pretendOnError)
	{
		if (!stream->ExpectDelimiter('{', token, EXPECTED_RIGHT_BRACE) && !pretendOnError)
		{
			return false;
		}
		EnterScopeInternal(type, parent);
		return true;
	}

	void Parser::RestoreFromPoint()
	{
		stream->RestoreFromPoint();
		modifierList = {};
		attribute.Reset();
	}

	bool Parser::EnterScope(ScopeType type, ASTNode* parent, bool pretendOnError)
	{
		Token token;
		ERR_IF_RETURN_FALSE(!EnterScope(type, parent, token, pretendOnError));
		return true;
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

	bool Parser::IterateScope(ASTNode* parent)
	{
		if (stream->TryGetDelimiter('}'))
		{
			ExitScopeInternal(parent);
			return false;
		}

		if (stream->IsEndOfTokens())
		{
			Log(EXPECTED_RIGHT_BRACE, stream->LastToken());
			ExitScopeInternal(parent);
			return false;
		}

		return true;
	}

	bool Parser::TryRecoverScope(ASTNode* parent, bool exitScope)
	{
		RestoreFromPoint();
		if (lastRecovery == stream->TokenPosition())
		{
			stream->Advance();
		}

		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			auto current = stream->Current();
			if (current.isKeywordOf(scopeRecoveryPoints))
			{
				Log(EXPECTED_RIGHT_BRACE, current);
				if (exitScope)
				{
					ExitScopeInternal(parent);
				}
				return false;
			}
			if (current.isKeywordOf(BuiltInTypes) || current.isKeywordOf(flowControlKeywords) || current.isIdentifier())
			{
				lastRecovery = stream->TokenPosition();
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
		'{', '}', ';',  '.', '$', '#', '('
	};

	static const std::unordered_set<char> parameterListContinueRecoveryPoints =
	{
		')', ','
	};

	bool Parser::TryRecoverParameterList()
	{
		if (lastRecovery == stream->TokenPosition())
		{
			stream->Advance();
		}

		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			auto current = stream->Current();
			if (current.isIdentifier() || current.isNumeric() || current.isKeywordOf(parameterFlagKeywords) || current.isKeywordOf(interpolationModifierKeywords))
			{
				lastRecovery = stream->TokenPosition();
				return true;
			}
			if (current.isKeyword() || current.isOperator() || current.isDelimiterOf(parameterListExitRecoveryPoints))
			{
				return false;
			}
			stream->Advance();
			if (stream->Current().isDelimiterOf(parameterListContinueRecoveryPoints))
			{
				lastRecovery = stream->TokenPosition();
				return true;
			}
		}

		return false;
	}

	bool Parser::TryRecoverParameterListMacro(bool inDefinition)
	{
		if (lastRecovery == stream->TokenPosition())
		{
			stream->Advance();
		}

		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			auto current = stream->Current();
			if (current.isIdentifier() || (current.isNumeric() && !inDefinition))
			{
				lastRecovery = stream->TokenPosition();
				return true;
			}
			if (current.isKeyword() || current.isOperator() || current.isDelimiterOf(parameterListExitRecoveryPoints) || current.isNewLine())
			{
				return false;
			}
			stream->Advance();
			if (stream->Current().isDelimiterOf(parameterListContinueRecoveryPoints))
			{
				lastRecovery = stream->TokenPosition();
				return true;
			}
		}

		return false;
	}

	void Parser::TryRecoverStatement()
	{
		if (lastRecovery == stream->TokenPosition())
		{
			stream->Advance();
		}

		while (!stream->IsEndOfTokens() && !stream->HasCriticalErrors())
		{
			auto current = stream->Current();
			if (current.isKeyword() || current.isDelimiterOf(';'))
			{
				lastRecovery = stream->TokenPosition();
				return;
			}
			stream->Advance();
		}
	}

	IdentifierInfo* Parser::ParseQualifiedName(bool& hasDot)
	{
		TextSpan identifier;
		stream->ExpectIdentifier(identifier, EXPECTED_IDENTIFIER);
		hasDot = false;
		while (stream->TryGetDelimiter('.'))
		{
			hasDot = true;
			TextSpan secondary;
			stream->ExpectIdentifier(secondary, EXPECTED_IDENTIFIER);
			identifier = identifier.merge(secondary);
		}

		return ASTContext::GetCurrentContext()->GetIdentifier(identifier.span());
	}

	NamespaceDeclaration Parser::ParseNamespaceDeclaration(bool& scoped)
	{
		auto nsKeywordSpan = stream->LastToken().Span;
		bool hasDot;
		auto name = ParseQualifiedName(hasDot);
		if (TryEnterScope(ScopeType_Namespace, nullptr))
		{
			scoped = true;
		}
		else
		{
			scoped = false;
			stream->ExpectDelimiter(';', EXPECTED_SEMICOLON);
		}

		return NamespaceDeclaration(nsKeywordSpan.merge(stream->LastToken().Span), name);
	}

	bool Parser::TryAdvance()
	{
		if (stream->Current().Type == TokenType_Unknown || stream->HasCriticalErrors())
		{
			return false;
		}

		return true;
	}

	bool Parser::Parse(CompilationUnitBuilder& builder)
	{
		stream->Advance();
		while (TryAdvance())
		{
			ASTNode* decl;
			if (!ParseSubStepInner(decl))
			{
				Log(UNEXPECTED_TOKEN, stream->Current());
				stream->TryAdvance();
			}
			else
			{
				builder.GetBuilder().AddDeclaration(decl);
			}
		}

		return !stream->HasCriticalErrors();
	}

	void Parser::ParseInnerBegin()
	{
		stream->MakeRestorePoint();
		ParseAttribute();
		ParseModifierList();
	}

	bool Parser::ParseSubStepInner(ASTNode*& declOut)
	{
		ParseInnerBegin();
		if (SubParserRegistry::TryParse(*this, *stream, declOut))
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
					Log(PUB_CANNOT_COMBINE_WITH_PRIV_PROT_INT, start.Span);
					hadError = true;
				}
				modifier |= AccessModifier_Public;
				hasPublic = true;
			}
			else if (stream->TryGetKeyword(Keyword_Internal))
			{
				if (hasPrivate || hasPublic)
				{
					Log(INT_CANNOT_COMBINE_WITH_PRIV_OR_PUB, start.Span);
					hadError = true;
				}
				modifier |= AccessModifier_Internal;
				hasInternal = true;
			}
			else if (stream->TryGetKeyword(Keyword_Protected))
			{
				if (hasPrivate || hasPublic)
				{
					Log(PROT_CANNOT_COMBINE_WITH_PRIV_OR_PUB, start.Span);
					hadError = true;
				}
				modifier |= AccessModifier_Protected;
				hasProtected = true;
			}
			else if (stream->TryGetKeyword(Keyword_Private))
			{
				if (hasPublic || hasProtected || hasInternal)
				{
					Log(PRIV_CANNOT_COMBINE_WITH_PUB_PROT_INT, start.Span);
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
					Log(IN_CANNOT_COMBINE_WITH_OUT_OR_INOUT, start.Span);
					hadError = true;
				}
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_In);
				hasIn = true;
			}
			else if (stream->TryGetKeyword(Keyword_Out))
			{
				if (hasIn || hasInOut)
				{
					Log(OUT_CANNOT_COMBINE_WITH_IN_OR_INOUT, start.Span);
					hadError = true;
				}
				flags = static_cast<ParameterFlags>(flags | ParameterFlags_Out);
				hasOut = true;
			}
			else if (stream->TryGetKeyword(Keyword_Inout))
			{
				if (hasIn || hasOut)
				{
					Log(INOUT_CANNOT_COMBINE_WITH_IN_OR_OUT, start.Span);
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
					Log(LINEAR_CANNOT_COMBINE_WITH_CENTROID_OR_NOINTERP, start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_Linear;
				hasLinear = true;
			}
			else if (stream->TryGetKeyword(Keyword_Centroid))
			{
				if (hasLinear || hasNoInterpolation)
				{
					Log(CENTROID_CANNOT_COMBINE_WITH_LINEAR_OR_NOINTERP, start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_Centroid;
				hasCentroid = true;
			}
			else if (stream->TryGetKeyword(Keyword_NoInterpolation))
			{
				if (hasLinear || hasCentroid || hasSample || hasNoPerspective)
				{
					Log(NOINTERP_CANNOT_COMBINE_WITH_OTHER_INTERP_MODIFIERS, start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_NoInterpolation;
				hasNoInterpolation = true;
			}
			else if (stream->TryGetKeyword(Keyword_Noperspective))
			{
				if (hasNoInterpolation)
				{
					Log(NOPERSPECTIVE_CANNOT_COMBINE_WITH_NOINTERP, start.Span);
					hadError = true;
				}
				modifier = InterpolationModifier_NoPerspecitve;
				hasNoPerspective = true;
			}
			else if (stream->TryGetKeyword(Keyword_Sample))
			{
				if (hasNoInterpolation)
				{
					Log(SAMPLE_CANNOT_COMBINE_WITH_NOINTERP, start.Span);
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

		SymbolRef* symbol;
		IF_ERR_RET(ParseSymbol(SymbolRefType_Attribute, symbol));

		

		std::vector<Expression*> parameters;
		if (stream->TryGetDelimiter('('))
		{
			bool firstParam = true;
			while (!stream->TryGetDelimiter(')'))
			{
				if (!firstParam)
				{
					IF_ERR_RET(stream->ExpectDelimiter(',', EXPECTED_COMMA));
				}
				firstParam = false;
				Expression* parameter;
				if (!HybridExpressionParser::ParseExpression(*this, *stream, parameter))
				{
					return;
				}
				parameters.push_back(std::move(parameter));
			}
		}

		IF_ERR_RET(stream->ExpectDelimiter(']', EXPECTED_RIGHT_BRACKET));
		auto span = stream->MakeFromLast(start);
		auto attribute = AttributeDecl::Create(span, symbol, parameters);
		this->attribute.Reset(attribute);
	}
}