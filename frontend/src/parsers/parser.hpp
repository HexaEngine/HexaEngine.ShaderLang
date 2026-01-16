#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexical/token_stream.hpp"
#include "lexical/text_span.hpp"
#include "pch/ast.hpp"
#include "logging/logger_adapter.hpp"
#include "lazy.hpp"

namespace HXSL
{
#define ERR_RETURN_FALSE(state, code) \
	do { \
		state.Log(code, state.GetStream().Current()); \
		return false; \
	} while (0)

#define ERR_RETURN_DEFAULT(state, code) \
	do { \
		state.Log(code); \
		return { }; \
	} while (0)

#define ERR_IF_RETURN_DEFAULT(r) \
	if (r) { \
		return { }; \
	}

#define ERR_IF_RETURN_FALSE(r) \
	if (r) { \
		return false; \
	}

	enum ScopeType
	{
		ScopeType_Global,
		ScopeType_Namespace,
		ScopeType_Struct,
		ScopeType_Class,
		ScopeType_Function,
		ScopeType_If,
		ScopeType_Else,
		ScopeType_ElseIf,
		ScopeType_While,
		ScopeType_For,
		ScopeType_Switch,
		ScopeType_Initialization,
		ScopeType_Unknown,
	};

	enum ScopeFlags
	{
		ScopeFlags_None = 0,
		ScopeFlags_InsideNamespace = 1 << 0,
		ScopeFlags_InsideStruct = 1 << 1,
		ScopeFlags_InsideClass = 1 << 2,
		ScopeFlags_InsideSwitch = 1 << 3,
		ScopeFlags_InsideFunction = 1 << 4,
		ScopeFlags_InsideIf = 1 << 5,
		ScopeFlags_InsideElse = 1 << 6,
		ScopeFlags_InsideElseIf = 1 << 7,
		ScopeFlags_InsideGlobal = 1 << 8,
		ScopeFlags_InsideWhile = 1 << 9,
		ScopeFlags_InsideFor = 1 << 10,
		ScopeFlags_InsideInitialization = 1 << 11,
		ScopeFlags_InsideLoop = ScopeFlags_InsideWhile | ScopeFlags_InsideFor,
	};

	inline static ScopeFlags operator|(ScopeFlags lhs, ScopeFlags rhs) {
		return static_cast<ScopeFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	inline static ScopeFlags operator&(ScopeFlags lhs, ScopeFlags rhs) {
		return static_cast<ScopeFlags>(static_cast<int>(lhs) & static_cast<int>(rhs));
	}

	inline static ScopeFlags operator~(ScopeFlags lhs) {
		return static_cast<ScopeFlags>(~static_cast<int>(lhs));
	}

	inline static ScopeFlags& operator|=(ScopeFlags& lhs, ScopeFlags rhs) {
		lhs = lhs | rhs;
		return lhs;
	}

	inline static ScopeFlags& operator&=(ScopeFlags& lhs, ScopeFlags rhs) {
		lhs = lhs & rhs;
		return lhs;
	}

	struct ParserScopeContext
	{
		ScopeType Type;
		ASTNode* Parent;
		ScopeFlags Flags;

		ParserScopeContext() : Type(ScopeType_Global), Parent(nullptr), Flags(ScopeFlags_None) {}
		ParserScopeContext(ScopeType type, ASTNode* parent, ScopeFlags flags) : Type(type), Parent(parent), Flags(flags) {}
	};

	template <typename T>
	struct TakeHandle
	{
	private:
		T* ptr = nullptr;

	public:
		explicit TakeHandle(T* resource)
			: ptr(resource)
		{
		}

		TakeHandle() = default;

		bool HasResource() const
		{
			return static_cast<bool>(ptr);
		}

		T* Get()
		{
			return ptr;
		}

		const T* Get() const {
			return ptr;
		}

		T* Take() {
			return std::move(ptr);
		}

		void Reset() {
			ptr = nullptr;
		}

		void Reset(T* new_ptr)
		{
			ptr = new_ptr;
		}
	};

	struct ModifierList
	{
		AccessModifier accessModifiers = AccessModifier_Private;
		bool anyAccessModifiersSpecified = false;
		FunctionFlags functionFlags = FunctionFlags_None;
		StorageClass storageClasses = StorageClass_None;
		InterpolationModifier interpolationModifiers = InterpolationModifier_Linear;
		bool anyInterpolationModifiersSpecified = false;
		TextSpan span;

		bool Empty() const noexcept
		{
			return !anyAccessModifiersSpecified && functionFlags == FunctionFlags_None && storageClasses == StorageClass_None && !anyInterpolationModifiersSpecified;
		}

		ModifierList(const AccessModifier& accessModifiers = AccessModifier_Private, bool anyAccessModifiersSpecified = false, const FunctionFlags& functionFlags = FunctionFlags_None, const StorageClass& storageClasses = StorageClass_None, const InterpolationModifier& interpolationModifiers = InterpolationModifier_Linear, bool anyInterpolationModifiersSpecified = false)
			: accessModifiers(accessModifiers), anyAccessModifiersSpecified(anyAccessModifiersSpecified), functionFlags(functionFlags), storageClasses(storageClasses), interpolationModifiers(interpolationModifiers), anyInterpolationModifiersSpecified(anyInterpolationModifiersSpecified)
		{
		}
	};

	static const std::unordered_set<Keyword> BuiltInTypes =
	{
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

	static const std::unordered_set<Keyword> KeywordLiterals =
	{
		Keyword_True,
		Keyword_False,
	};

	class Parser : public LoggerAdapter
	{
	public:
		TokenStream* stream;
		int ScopeLevel;
		int NamespaceScope;
		ParserScopeContext CurrentScope;
		std::stack<ParserScopeContext> ScopeStack;
		TakeHandle<AttributeDecl> attribute;
		ModifierList modifierList;
		size_t lastRecovery;

		Parser() = default;
		Parser(ILogger* logger, TokenStream& stream) : LoggerAdapter(logger), stream(&stream), ScopeLevel(0), NamespaceScope(0), CurrentScope(ParserScopeContext(ScopeType_Global, nullptr, ScopeFlags_None)), modifierList({}), lastRecovery(-1)
		{
		}

		void static InitializeSubSystems();

		IdentifierTable& GetIdentifierTable() noexcept { return ASTContext::GetCurrentContext()->GetIdentifierTable(); }

		TokenStream& GetStream() noexcept { return *stream; }

		int scopeLevel() const noexcept { return ScopeLevel; }

		ScopeType scopeType() const noexcept { return CurrentScope.Type; }

		ASTNode* scopeParent() const noexcept { return CurrentScope.Parent; }

		ScopeFlags scopeFlags() const noexcept { return CurrentScope.Flags; }

		template<typename... Args>
		bool inScope(ScopeFlags flags, DiagnosticCode code, Args&&... args) const noexcept
		{
			if (!(scopeFlags() & flags))
			{
				Log(code, stream->LastToken(), std::forward<Args>(args)...);
				return false;
			}

			return true;
		}

		bool IsInNamespaceScope(bool strict = false) const
		{
			return strict ? ScopeLevel == NamespaceScope : ScopeLevel >= NamespaceScope;
		}

		bool IsInGlobalOrNamespaceScope() const
		{
			return ScopeLevel == NamespaceScope || ScopeLevel == 0;
		}

		void EnterScopeInternal(ScopeType type, ASTNode* userdata);
		void ExitScopeInternal(ASTNode* parent);
		bool TryEnterScope(ScopeType type, ASTNode* parent);

		bool EnterScope(ScopeType type, ASTNode* parent, Token& token, bool pretentOnError);

		template<typename... Args>
		bool EnterScope(ScopeType type, ASTNode* parent, Token& token, bool pretentOnError, DiagnosticCode code, Args&&... args)
		{
			if (!stream->ExpectDelimiter('{', token, code, std::forward<Args>(args)...) && !pretentOnError)
			{
				return false;
			}
			EnterScopeInternal(type, parent);
			return true;
		}

		void RestoreFromPoint();
		bool EnterScope(ScopeType type, ASTNode* parent, bool pretendOnError);
		bool IterateScope(ASTNode* parent);
		bool TryRecoverScope(ASTNode* parent, bool exitScope);
		bool TryRecoverParameterList();
		bool TryRecoverParameterListMacro(bool inDefinition);
		void TryRecoverStatement();
		IdentifierInfo* ParseQualifiedName(bool& hasDot);
		NamespaceDeclaration ParseNamespaceDeclaration(bool& scoped);
		bool TryAdvance();
		bool Parse(CompilationUnitBuilder& builder);
		void ParseInnerBegin();
		bool ParseSubStepInner(ASTNode*& declOut);
		bool AttemptErrorRecovery(bool restorePoint = false);

		/// <summary>
		/// This method checks if an attribute is available and allows the subparser to decide whether to accept it.
		/// If an attribute is found and the subparser does not accept it (i.e., `nullptr` is passed), the method returns `false`
		/// to indicate that the subparser should not continue further processing. If an attribute is found and the subparser
		/// accepts it (i.e., `attributeOut` is not `nullptr`), the attribute will be passed back to the subparser via the `attributeOut` parameter.
		/// If no attribute is found, the method returns `true`, allowing the parser to continue normally regardless of the input.
		/// </summary>
		/// <param name="attributeOut">
		/// An output parameter that will hold the attribute if it is accepted by the subparser. If `nullptr` is passed,
		/// the subparser indicates that it does not accept the attribute and no attribute will be passed back.
		/// </param>
		/// <returns>
		/// Returns `true` if no attribute is present (allowing the parser to continue), regardless of whether `nullptr`
		/// or a valid pointer is passed. Returns `true` if the attribute is successfully passed to the subparser.
		/// Returns `false` if an attribute exists but is not accepted by the subparser (i.e., `nullptr` is passed for `attributeOut`),
		/// signaling that the subparser should not continue.
		/// </returns>
		/// <remarks>
		/// This method is commonly used in subparsers to check for attributes. Passing `nullptr` means the subparser does not
		/// accept the attribute, while passing a valid output parameter allows the subparser to accept the attribute and
		/// continue processing. If the subparser does not accept the attribute but one is found, the method returns `false`
		/// to indicate that the subparser should not continue further processing. If no attribute is found, the method returns
		/// `true` to continue normally, regardless of the input.
		/// </remarks>
		template<typename... Args>
		bool AcceptAttribute(TakeHandle<AttributeDecl>** attributeOut, DiagnosticCode code, Args&&... args)
		{
			if (!attribute.Get())
			{
				if (attributeOut)
				{
					attributeOut = nullptr;
				}
				return true;
			}

			if (attributeOut)
			{
				*attributeOut = &attribute;
				return true;
			}

			Log(code, attribute.Get()->GetSpan(), attribute.Get()->GetSpan().str(), std::forward<Args>(args)...);
			attribute.Reset();
			return false;
		}

		template<typename... Args>
		bool RejectAttribute(DiagnosticCode code, Args&&... args)
		{
			return AcceptAttribute(nullptr, code, std::forward<Args>(args)...);
		}

		template<typename... Args>
		bool AcceptModifierList(ModifierList* modifierListOut, const ModifierList& allowed, DiagnosticCode code, Args&&... args)
		{
			bool isValid = true;

			std::string format;
			auto& span = modifierList.span;
			if (modifierListOut == nullptr)
			{
				if (modifierList.anyAccessModifiersSpecified)
				{
					Log(code, span, std::forward<Args>(args)...);
					isValid = false;
				}
				if (modifierList.functionFlags != FunctionFlags_None)
				{
					Log(code, span, std::forward<Args>(args)...);
					isValid = false;
				}
				if (modifierList.storageClasses != StorageClass_None)
				{
					Log(code, span, std::forward<Args>(args)...);
					isValid = false;
				}
				if (modifierList.anyInterpolationModifiersSpecified)
				{
					Log(code, span, std::forward<Args>(args)...);
					isValid = false;
				}

				modifierList = {};
				return false;
			}

			if (!allowed.anyAccessModifiersSpecified && modifierList.anyAccessModifiersSpecified)
			{
				Log(code, span, std::forward<Args>(args)...);
				isValid = false;
			}
			else if (modifierList.anyAccessModifiersSpecified && (modifierList.accessModifiers & ~allowed.accessModifiers) != 0)
			{
				Log(code, span, std::forward<Args>(args)...);
				isValid = false;
			}
			if ((modifierList.functionFlags & ~allowed.functionFlags) != 0)
			{
				Log(code, span, std::forward<Args>(args)...);
				isValid = false;
			}
			if ((modifierList.storageClasses & ~allowed.storageClasses) != 0)
			{
				Log(code, span, std::forward<Args>(args)...);
				isValid = false;
			}
			if (!allowed.anyInterpolationModifiersSpecified && modifierList.anyInterpolationModifiersSpecified)
			{
				Log(code, span, std::forward<Args>(args)...);
				isValid = false;
			}
			else if (modifierList.anyInterpolationModifiersSpecified && (modifierList.interpolationModifiers & ~allowed.interpolationModifiers) != 0)
			{
				Log(code, span, std::forward<Args>(args)...);
				isValid = false;
			}

			if (isValid)
			{
				*modifierListOut = modifierList;
			}

			modifierList = {};

			return isValid;
		}

		template<typename... Args>
		bool RejectModifierList(DiagnosticCode code, Args&&... args)
		{
			return AcceptModifierList(nullptr, {}, code, std::forward<Args>(args)...);
		}

		std::tuple<ParameterFlags, InterpolationModifier> ParseParameterFlags();
		bool TryParseSymbol(const SymbolRefType& type, LazySymbol& symbol);
		bool ParseSymbol(SymbolRefType expectedType, SymbolRef*& type);

		bool TryParseArraySizes(std::vector<size_t>& arraySizes);
		void ParseArraySizes(std::vector<size_t>& arraySizes);
	private:
		friend class ParserHelper;
		friend class MemberPathParser;
		bool TryParseSymbolInternal(const SymbolRefType& type, TextSpan& span);
		void ParseAttribute();
		void ParseModifierList();
		AccessModifier ParseAccessModifiers(bool& anySpecified);
		StorageClass ParseStorageClasses();
		InterpolationModifier ParseInterpolationModifiers(bool& anySpecified);
		FunctionFlags ParseFunctionFlags();
	};

	struct ParserAdapter
	{
	protected:
		Parser& parser;

	public:
		ParserAdapter(Parser& p) : parser(p) {}

		Parser& GetParser()
		{
			return parser;
		}

		TokenStream& GetStream() noexcept { return parser.GetStream(); }
		int GetScopeLevel() const noexcept { return parser.scopeLevel(); }
		ScopeType GetScopeType() const noexcept { return parser.scopeType(); }
		ASTNode* GetScopeParent() const noexcept { return parser.scopeParent(); }
		ScopeFlags GetScopeFlags() const noexcept { return parser.scopeFlags(); }

		template<typename... Args>
		void Log(DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			parser.Log(code, span, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogIf(bool condition, DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			parser.LogIf(condition, code, span, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void Log(DiagnosticCode code, const Token& token, Args&&... args) const
		{
			parser.Log(code, token, std::forward<Args>(args)...);
		}

		template<typename... Args>
		bool inScope(ScopeFlags flags, DiagnosticCode code, Args&&... args) const noexcept
		{
			return parser.inScope(flags, std::forward<Args>(args)...);
		}

		bool IsInNamespaceScope(bool strict = false) const { return parser.IsInNamespaceScope(strict); }
		bool IsInGlobalOrNamespaceScope() const { return parser.IsInGlobalOrNamespaceScope(); }

		void RestoreFromPoint() { parser.RestoreFromPoint(); }

		bool EnterScope(ScopeType type, ASTNode* parent, bool pretendOnError)
		{
			return parser.EnterScope(type, parent, pretendOnError);
		}

		bool EnterScope(ScopeType type, ASTNode* parent, Token& token, bool pretendOnError)
		{
			return parser.EnterScope(type, parent, token, pretendOnError);
		}

		template<typename... Args>
		bool EnterScope(ScopeType type, ASTNode* parent, Token& token, bool pretendOnError, const std::string& message, Args&&... args)
		{
			return parser.EnterScope(type, parent, token, pretendOnError, message, std::forward<Args>(args)...);
		}

		bool IterateScope(ASTNode* parent) { return parser.IterateScope(parent); }
		bool TryRecoverScope(ASTNode* parent, bool exitScope) { return parser.TryRecoverScope(parent, exitScope); }
		bool TryRecoverParameterList() { return parser.TryRecoverParameterList(); }
		void TryRecoverStatement() { parser.TryRecoverStatement(); }

		bool TryAdvance() { return parser.TryAdvance(); }

		void ParseInnerBegin() { parser.ParseInnerBegin(); }
		bool ParseSubStepInner(ASTNode*& declOut) { return parser.ParseSubStepInner(declOut); }
		bool AttemptErrorRecovery(bool restorePoint = false) { return parser.AttemptErrorRecovery(restorePoint); }

		template<typename... Args>
		bool AcceptAttribute(TakeHandle<AttributeDecl>** attributeOut, DiagnosticCode code, Args&&... args)
		{
			return parser.AcceptAttribute(attributeOut, code, std::forward<Args>(args)...);
		}

		template<typename... Args>
		bool RejectAttribute(DiagnosticCode code, Args&&... args)
		{
			return parser.RejectAttribute(code, std::forward<Args>(args)...);
		}

		template<typename... Args>
		bool AcceptModifierList(ModifierList* modifierListOut, const ModifierList& allowed, DiagnosticCode code, Args&&... args)
		{
			return parser.AcceptModifierList(modifierListOut, allowed, code, std::forward<Args>(args)...);
		}

		template<typename... Args>
		bool RejectModifierList(DiagnosticCode code, Args&&... args)
		{
			return parser.RejectModifierList(code, std::forward<Args>(args)...);
		}

		std::tuple<ParameterFlags, InterpolationModifier> ParseParameterFlags() { return parser.ParseParameterFlags(); }

		bool TryParseSymbol(const SymbolRefType& type, LazySymbol& symbol) { return parser.TryParseSymbol(type, symbol); }

		bool ParseSymbol(SymbolRefType expectedType, SymbolRef*& type) { return parser.ParseSymbol(expectedType, type); }

		bool TryParseArraySizes(std::vector<size_t>& arraySizes) { return parser.TryParseArraySizes(arraySizes); }

		void ParseArraySizes(std::vector<size_t>& arraySizes) { parser.ParseArraySizes(arraySizes); }
	};
}
#endif