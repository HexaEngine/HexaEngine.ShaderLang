#ifndef PARSERS_LAZY_HPP
#define PARSERS_LAZY_HPP

#include "ast_modules/symbol_base.hpp"

namespace HXSL
{
	struct LazySymbol
	{
		TextSpan span;
		SymbolRefType type;
		bool fqn;
		SymbolRef* ptr;

		LazySymbol() : span({}), type(SymbolRefType_Unknown), fqn(false)
		{
		}

		LazySymbol(SymbolRef* ptr) : span({}), type(SymbolRefType_Unknown), ptr(ptr), fqn(false)
		{
		}

		LazySymbol(TextSpan span, SymbolRefType type, bool fqn) : span(span), type(type), fqn(fqn)
		{
		}

		SymbolRef* make(SymbolRefType overwrite, bool force = false)
		{
			if (ptr)
			{
				return ptr;
			}

			if (span.source == INVALID_SOURCE_ID && !force)
			{
				HXSL_ASSERT(false, "Attempted to double create a LazySymbol");
				return {};
			}
			auto result = SymbolRef::Create(span, ASTContext::GetCurrentContext()->GetIdentifier(span), overwrite, fqn);
			span = {};
			type = {};
			return result;
		};

		SymbolRef* make(bool force = false)
		{
			return make(type, force);
		};

		SymbolRefArray* MakeArrayRef(const Span<size_t>& arrayDims, bool force = false)
		{
			if (span.source == INVALID_SOURCE_ID)
			{
				HXSL_ASSERT(false, "Attempted to double create a LazySymbol");
				return {};
			}
			auto result = SymbolRefArray::Create(span, ASTContext::GetCurrentContext()->GetIdentifier(span), arrayDims);
			span = {};
			type = {};
			return result;
		};
	};
}

#endif