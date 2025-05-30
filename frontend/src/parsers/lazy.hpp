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
		ast_ptr<SymbolRef> ptr;

		LazySymbol() : span({}), type(SymbolRefType_Unknown), fqn(false)
		{
		}

		LazySymbol(ast_ptr<SymbolRef> ptr) : span({}), type(SymbolRefType_Unknown), ptr(std::move(ptr)), fqn(false)
		{
		}

		LazySymbol(TextSpan span, SymbolRefType type, bool fqn) : span(span), type(type), fqn(fqn)
		{
		}

		ast_ptr<SymbolRef> make(SymbolRefType overwrite, bool force = false)
		{
			if (ptr.get())
			{
				return std::move(ptr);
			}

			if (span.source == nullptr && !force)
			{
				HXSL_ASSERT(false, "Attempted to double create a LazySymbol");
				return {};
			}
			auto result = make_ast_ptr<SymbolRef>(span, overwrite, fqn);
			span = {};
			type = {};
			return std::move(result);
		};

		ast_ptr<SymbolRef> make(bool force = false)
		{
			return std::move(make(type, force));
		};
	};
}

#endif