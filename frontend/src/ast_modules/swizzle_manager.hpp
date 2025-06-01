#ifndef SWIZZLE_MANAGER_HPP
#define SWIZZLE_MANAGER_HPP

#include "swizzle.hpp"
#include "primitive_manager.hpp"
#include "semantics/symbols/symbol_table.hpp"

namespace HXSL
{
	class SwizzleManager
	{
	private:
		ASTContext* context;
		std::unique_ptr<SymbolTable> swizzleTable = std::make_unique<SymbolTable>();
		PrimitiveManager& primitives;

		static char NormalizeSwizzleChar(const char& c)
		{
			switch (c) {
			case 'r': return 'x';
			case 'g': return 'y';
			case 'b': return 'z';
			case 'a': return 'w';
			case 's': return 'x';
			case 't': return 'y';
			case 'p': return 'z';
			case 'q': return 'w';
			default: return c;
			}
		}
		static int SwizzleCharToIndex(char c)
		{
			switch (c) {
			case 'x': return 0;
			case 'y': return 1;
			case 'z': return 2;
			case 'w': return 3;
			default: return -1;
			}
		}

	public:
		SwizzleManager(ASTContext& context, PrimitiveManager& primitives) : context(&context), primitives(primitives)
		{
		}

		bool VerifySwizzle(Primitive* prim, SymbolRef* ref);
	};
}

#endif