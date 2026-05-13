#include "hxls_compiler.hpp"
#include <utils/co_trampoline.hpp>

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

using namespace HXSL;

static size_t nextTask = 0;

TrampolineTask<bool> Test2()
{
	nextTask = 1;
	co_await TrampolineBounce();
	nextTask = 22;
	co_await TrampolineBounce();
	co_return true;
}

TrampolineTask<bool> Test()
{
	if (nextTask == 22) co_return true;
	co_await Test2();
	co_return true;
}




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