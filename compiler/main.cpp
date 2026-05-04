#include "hxls_compiler.hpp"

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

using namespace HXSL;

int main()
{
	SetLocale("en_US");

	Compiler compiler = Compiler();
	compiler.Compile({ "example/library.txt" }, "library.hlib");

	std::vector<AssemblyReference> refs = { { "library.hlib" } };
	compiler.Compile({ "example/shader.txt" }, "test.hlib", refs);

	//_CrtDumpMemoryLeaks();
	return 0;
}