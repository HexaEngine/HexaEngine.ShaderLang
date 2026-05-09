#include "hxls_compiler.hpp"

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

using namespace HXSL;

int main()
{
	SetLocale("en_US");

	Compiler compiler = Compiler();

	std::cout << "Compiling library2.hlib" << std::endl;
	compiler.Compile({ "example/library2.txt" }, "library2.hlib");

	std::cout << "Compiling library.hlib" << std::endl;
	std::vector<AssemblyReference> refsLib = { { "library2.hlib" } };
	compiler.Compile({ "example/library.txt" }, "library.hlib", refsLib);

	std::cout << "Compiling test.hlib" << std::endl;
	std::vector<AssemblyReference> refs = { { "library.hlib" } };
	compiler.Compile({ "example/shader.txt" }, "test.hlib", refs);

	//_CrtDumpMemoryLeaks();
	return 0;
}