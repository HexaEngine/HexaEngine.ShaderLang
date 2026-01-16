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

		static ASTContext*& GetCurrentContextStorage()
		{
			static thread_local ASTContext* currentContext = nullptr;
			return currentContext;
		}

	public:
		BumpAllocator& GetAllocator() { return allocator; }
		SourceManager& GetSourceManager() { return sourceManager; }
		IdentifierTable& GetIdentifierTable() { return identifierTable; }

		~ASTContext()
		{
			if (GetCurrentContext() == this)
			{
				SetCurrentContext(nullptr);
			}
		}

		IdentifierInfo* GetIdentifier(const TextSpan& span)
		{
			return identifierTable.Get(sourceManager.GetSpan(span));
		}

		IdentifierInfo* GetIdentifier(const StringSpan& span)
		{
			return identifierTable.Get(span);
		}

		template<typename T, typename...Args>
		T* Alloc(size_t size, Args&&... args)
		{
			T* ptr = reinterpret_cast<T*>(allocator.Alloc(size, alignof(T)));
			new (ptr) T(std::forward<Args>(args)...);
			return ptr;
		}

		template<typename T>
		Span<T> AllocCopy(const Span<T>& span)
		{
			return allocator.CopySpan(span);
		}

		static ASTContext* GetCurrentContext()
		{
			return GetCurrentContextStorage();
		}

		static void SetCurrentContext(ASTContext* context)
		{
			GetCurrentContextStorage() = context;
		}
	};
}

#endif