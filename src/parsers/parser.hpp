#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexical/token_stream.h"
#include "lexical/text_span.hpp"
#include "ast.hpp"
#include "lazy.hpp"
#include "utils/object_pool.hpp"

#include <stack>
#include <string>
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

	static TextSpan ParseQualifiedName(TokenStream& stream, bool& hasDot)
	{
		TextSpan identifier;
		stream.ExpectIdentifier(identifier, EXPECTED_IDENTIFIER);
		hasDot = false;
		while (stream.TryGetDelimiter('.'))
		{
			hasDot = true;
			TextSpan secondary;
			stream.ExpectIdentifier(secondary, EXPECTED_IDENTIFIER);
			identifier = identifier.merge(secondary);
		}

		return identifier;
	}

	template <typename T>
	struct TakeHandle
	{
	private:
		std::unique_ptr<T> ptr;

	public:
		explicit TakeHandle(std::unique_ptr<T> resource)
			: ptr(std::move(resource))
		{
		}

		TakeHandle() = default;

		bool HasResource() const
		{
			return static_cast<bool>(ptr);
		}

		T* Get()
		{
			return ptr.get();
		}

		const T* Get() const {
			return ptr.get();
		}

		std::unique_ptr<T> Take() {
			return std::move(ptr);
		}

		void Reset() {
			ptr.reset();
		}

		void Reset(std::unique_ptr<T> new_ptr)
		{
			ptr = std::move(new_ptr);
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

	class Parser
	{
	public:
		TokenStream* stream;
		int ScopeLevel;
		int NamespaceScope;
		Compilation* m_compilation;
		Namespace* CurrentNamespace;
		ASTNode* ParentNode;
		ParserScopeContext CurrentScope;
		std::stack<ParserScopeContext> ScopeStack;
		TakeHandle<AttributeDeclaration> attribute;
		ModifierList modifierList;
		size_t lastRecovery;

		Parser() = default;
		Parser(TokenStream& stream, Compilation* compilation) : stream(&stream), ScopeLevel(0), NamespaceScope(0), m_compilation(compilation), CurrentNamespace(nullptr), ParentNode(compilation), CurrentScope(ParserScopeContext(ScopeType_Global, compilation, ScopeFlags_None)), modifierList({}), lastRecovery(-1)
		{
		}

		void static InitializeSubSystems();

		Compilation* Compilation() const noexcept { return m_compilation; }

		TokenStream& GetStream() noexcept { return *stream; }

		int scopeLevel() const noexcept { return ScopeLevel; }

		ScopeType scopeType() const noexcept { return CurrentScope.Type; }

		ASTNode* scopeParent() const noexcept { return CurrentScope.Parent; }

		ScopeFlags scopeFlags() const noexcept { return CurrentScope.Flags; }

		template<typename... Args>
		void Log(DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			m_compilation->LogFormattedEx(code, " (Line: {}, Column: {})", std::forward<Args>(args)..., span.line, span.column);
		}

		template<typename... Args>
		void LogIf(bool condition, DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			if (condition)
			{
				m_compilation->LogFormattedEx(code, " (Line: {}, Column: {})", std::forward<Args>(args)..., span.line, span.column);
			}
		}

		template<typename... Args>
		void Log(DiagnosticCode code, const Token& token, Args&&... args) const
		{
			Log(code, token.Span, std::forward<Args>(args)...);
		}

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
		void TryRecoverStatement();
		UsingDeclaration ParseUsingDeclaration();
		NamespaceDeclaration ParseNamespaceDeclaration(bool& scoped);
		bool TryAdvance();
		bool Parse();
		bool ParseSubStep(ASTNode* parent);
		void ParseInnerBegin();
		bool ParseSubStepInner(ASTNode* parent);
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
		bool AcceptAttribute(TakeHandle<AttributeDeclaration>** attributeOut, DiagnosticCode code, Args&&... args)
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
		bool ParseSymbol(SymbolRefType expectedType, std::unique_ptr<SymbolRef>& type);

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

		// Direct accessors
		TokenStream& GetStream() noexcept { return parser.GetStream(); }
		Compilation* GetCompilation() const noexcept { return parser.Compilation(); }
		int GetScopeLevel() const noexcept { return parser.scopeLevel(); }
		ScopeType GetScopeType() const noexcept { return parser.scopeType(); }
		ASTNode* GetScopeParent() const noexcept { return parser.scopeParent(); }
		ScopeFlags GetScopeFlags() const noexcept { return parser.scopeFlags(); }

		// Logging
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

		UsingDeclaration ParseUsingDeclaration() { return parser.ParseUsingDeclaration(); }
		NamespaceDeclaration ParseNamespaceDeclaration(bool& scoped) { return parser.ParseNamespaceDeclaration(scoped); }

		bool TryAdvance() { return parser.TryAdvance(); }
		bool Parse() { return parser.Parse(); }

		bool ParseSubStep(ASTNode* parent) { return parser.ParseSubStep(parent); }
		void ParseInnerBegin() { parser.ParseInnerBegin(); }
		bool ParseSubStepInner(ASTNode* parent) { return parser.ParseSubStepInner(parent); }
		bool AttemptErrorRecovery(bool restorePoint = false) { return parser.AttemptErrorRecovery(restorePoint); }

		template<typename... Args>
		bool AcceptAttribute(TakeHandle<AttributeDeclaration>** attributeOut, DiagnosticCode code, Args&&... args)
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

		bool ParseSymbol(SymbolRefType expectedType, std::unique_ptr<SymbolRef>& type) { return parser.ParseSymbol(expectedType, type); }

		bool TryParseArraySizes(std::vector<size_t>& arraySizes) { return parser.TryParseArraySizes(arraySizes); }

		void ParseArraySizes(std::vector<size_t>& arraySizes) { parser.ParseArraySizes(arraySizes); }
	};
}
#endif