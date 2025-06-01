#ifndef AST_CONTEXT_HPP
#define AST_CONTEXT_HPP

#include "utils/bump_allocator.hpp"
#include "lexical/identifier_table.hpp"
#include "io/source_manager.hpp"

namespace HXSL
{
	class ASTContext
	{
		BumpAllocator allocator;
		SourceManager sourceManager;
		IdentifierTable identifierTable;

	public:
		BumpAllocator& GetAllocator() { return allocator; }
		SourceManager& GetSourceManager() { return sourceManager; }
		IdentifierTable& GetIdentiferTable() { return identifierTable; }

		template<typename T, typename...Args>
		T* Alloc(size_t size, Args&&... args)
		{
			T* ptr = reinterpret_cast<T*>(allocator.Alloc(size, alignof(T)));
			new (ptr) T(std::forward<Args>(args)...);
			return ptr;
		}
	};
}

#endif