#include "ast_allocator.hpp"

static thread_local ASTAllocator* allocator = new ASTAllocator();

ASTAllocator* GetThreadAllocator()
{
	return allocator;
}