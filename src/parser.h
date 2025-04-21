#ifndef PARSER_H
#define PARSER_H

#include "token_stream.h"
#include "compilation.hpp"
#include "text_span.h"
#include "nodes.hpp"
#include "lang/language.h"

#include <stack>
#include <string>
namespace HXSL
{
#define ERR_RETURN_FALSE(state, message) \
	do { \
		state.LogError(message); \
		return false; \
	} while (0)

#define ERR_RETURN_DEFAULT(state, message) \
	do { \
		state.LogError(message); \
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
		ScopeFlags_InsideSwitch = 1 << 2,
		ScopeFlags_InsideFunction = 1 << 3,
		ScopeFlags_InsideIf = 1 << 4,
		ScopeFlags_InsideElse = 1 << 5,
		ScopeFlags_InsideElseIf = 1 << 6,
		ScopeFlags_InsideGlobal = 1 << 7,
		ScopeFlags_InsideWhile = 1 << 8,
		ScopeFlags_InsideFor = 1 << 9,
		ScopeFlags_InsideInitialization = 1 << 10,
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

	struct ResolverScopeContext
	{
		TextSpan Name;
		ScopeType Type;
		ASTNode* Parent;
		ScopeFlags Flags;

		ResolverScopeContext() : Name({ }), Type(ScopeType_Global), Parent(nullptr), Flags(ScopeFlags_None) {}
		ResolverScopeContext(TextSpan name, ScopeType type, ASTNode* parent, ScopeFlags flags) : Name(name), Type(type), Parent(parent), Flags(flags) {}
	};

	static TextSpan ParseQualifiedName(TokenStream& stream, bool& hasDot)
	{
		TextSpan identifier;
		stream.ExpectIdentifier(identifier);
		hasDot = false;
		while (stream.TryGetDelimiter('.'))
		{
			hasDot = true;
			TextSpan secondary;
			stream.ExpectIdentifier(secondary);
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

	class HXSLParser
	{
	public:
		TokenStream& Stream;
		int ScopeLevel;
		int NamespaceScope;
		Compilation* m_compilation;
		HXSLNamespace* CurrentNamespace;
		ASTNode* ParentNode;
		ResolverScopeContext CurrentScope;
		std::stack<ResolverScopeContext> ScopeStack;
		std::stack<ASTNode*> ParentStack;
		TakeHandle<HXSLAttributeDeclaration> attribute;

		HXSLParser(TokenStream& stream, Compilation* compilation) : Stream(stream), ScopeLevel(0), NamespaceScope(0), m_compilation(compilation), CurrentNamespace(nullptr), ParentNode(compilation), CurrentScope(ResolverScopeContext({}, ScopeType_Global, compilation, ScopeFlags_None))
		{
		}

		Compilation* Compilation() const noexcept { return m_compilation; }

		TokenStream& stream() noexcept { return Stream; }

		int scopeLevel() const noexcept { return ScopeLevel; }

		ScopeType scopeType() const noexcept { return CurrentScope.Type; }

		TextSpan scopeName() const noexcept { return CurrentScope.Name; }

		ASTNode* scopeParent() const noexcept { return CurrentScope.Parent; }

		ScopeFlags scopeFlags() const noexcept { return CurrentScope.Flags; }

		ASTNode* parentNode() const noexcept { return ParentNode; }

		template<typename... Args>
		void LogError(const std::string& message, TextSpan span, Args&&... args) const
		{
			std::string format = message + " (Line: %i, Column: %i)";
			Stream.StreamState.State.logger()->LogFormatted(HXSLLogLevel_Error, format, std::forward<Args>(args)..., span.Line, span.Column);
		}

		template<typename... Args>
		void LogError(const std::string& message, Token token, Args&&... args) const
		{
			LogError(message, token.Span, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void LogError(const std::string& message, Args&&... args) const
		{
			LogError(message, Stream.Current().Span, std::forward<Args>(args)...);
		}

		bool inScope(ScopeFlags flags, const std::string& message) const noexcept
		{
			if (!(scopeFlags() & flags))
			{
				LogError(message, Stream.LastToken());
				return false;
			}

			return true;
		}

		bool inScope(ScopeFlags flags) const noexcept
		{
			return inScope(flags, "Invalid token in current scope.");
		}

		void pushParentNode(ASTNode* parent)
		{
			HXSL_ASSERT(parent != nullptr, "Parent node cannot be null.");
			ParentStack.push(ParentNode);
			ParentNode = parent;
		}

		void popParentNode()
		{
			if (ParentStack.empty())
			{
				HXSL_ASSERT(false, "Attempted to pop parent stack, but it's empty.");
				return;
			}
			ParentNode = ParentStack.top();
			ParentStack.pop();
		}

		bool IsInNamespaceScope(bool strict = false) const
		{
			return strict ? ScopeLevel == NamespaceScope : ScopeLevel >= NamespaceScope;
		}

		bool IsInGlobalOrNamespaceScope() const
		{
			return ScopeLevel == NamespaceScope || ScopeLevel == 0;
		}

		HXSLFieldFlags ParseFieldFlags();
		bool TryParseAttribute(std::unique_ptr<HXSLAttributeDeclaration>& attribute);
		void EnterScopeInternal(TextSpan name, ScopeType type, ASTNode* userdata);
		void ExitScopeInternal();
		bool TryEnterScope(TextSpan name, ScopeType type, ASTNode* parent);
		bool EnterScope(TextSpan name, ScopeType type, ASTNode* parent, Token& token);
		bool EnterScope(TextSpan name, ScopeType type, ASTNode* parent);
		bool SkipScope(Token& token);
		bool IterateScope();
		UsingDeclaration ParseUsingDeclaration();
		NamespaceDeclaration ParseNamespaceDeclaration(bool& scoped);
		bool TryAdvance();
		bool Parse();
		bool ParseSubStep();

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
		bool AcceptAttribute(TakeHandle<HXSLAttributeDeclaration>** attributeOut, const std::string& message, Args&&... args)
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

			std::string format = "Attribute '%s' " + message;
			LogError(format, attribute.Get()->GetSpan(), attribute.Get()->GetSpan().toString().c_str(), std::forward<Args>(args)...);
			attribute.Reset();
			return false;
		}

		template<typename... Args>
		bool RejectAttribute(const std::string& message, Args&&... args)
		{
			return AcceptAttribute(nullptr, message, std::forward<Args>(args)...);
		}

		HXSLAccessModifier ParseAccessModifier();
		bool TryParseSymbol(HXSLSymbolRefType expectedType, LazySymbol& type);
		bool ParseSymbol(HXSLSymbolRefType expectedType, std::unique_ptr<HXSLSymbolRef>& type);
		HXSLParameterFlags ParseParameterFlags();
		HXSLFunctionFlags ParseFunctionFlags();
	};
}
#endif