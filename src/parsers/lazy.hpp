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
		std::unique_ptr<SymbolRef> ptr;

		LazySymbol() : span({}), type(SymbolRefType_Unknown), fqn(false)
		{
		}

		LazySymbol(std::unique_ptr<SymbolRef> ptr) : span({}), type(SymbolRefType_Unknown), ptr(std::move(ptr)), fqn(false)
		{
		}

		LazySymbol(TextSpan span, SymbolRefType type, bool fqn) : span(span), type(type), fqn(fqn)
		{
		}

		std::unique_ptr<SymbolRef> make(SymbolRefType overwrite)
		{
			if (ptr.get())
			{
				return std::move(ptr);
			}

			if (span.Text == nullptr)
			{
				HXSL_ASSERT(false, "Attempted to double create a LazySymbol");
				return {};
			}
			auto result = std::make_unique<SymbolRef>(span, overwrite, fqn);
			span = {};
			type = {};
			return std::move(result);
		};

		std::unique_ptr<SymbolRef> make()
		{
			return std::move(make(type));
		};
	};
}

#endif