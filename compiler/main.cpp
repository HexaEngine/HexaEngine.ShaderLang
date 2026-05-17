#include "hxls_compiler.hpp"
#include <utils/co_trampoline.hpp>

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <core/options.hpp>

using namespace HXSL;
using namespace HXSL::Backend;

int main()
{
	OptionCollection options;

	SetLocale("en_US");

	Compiler compiler = Compiler(options);
	compiler.Compile({ "example/inline_multiblock.txt" }, "inline_multiblock.hlib");

	/*
	std::cout << "Compiling library2.hlib" << std::endl;
	compiler.Compile({ "example/library2.txt" }, "library2.hlib");

	std::cout << "Compiling library.hlib" << std::endl;
	std::vector<AssemblyReference> refsLib = { { "library2.hlib" } };
	compiler.Compile({ "example/library.txt" }, "library.hlib", refsLib);

	options.Set<InlinerInlineExtern>(true);

	std::cout << "Compiling test.hlib" << std::endl;
	std::vector<AssemblyReference> refs = { { "library.hlib" } };
	compiler.Compile({ "example/shader.txt" }, "test.hlib", refs);
	*/
	//_CrtDumpMemoryLeaks();
	return 0;
}