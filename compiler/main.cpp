#include "hxls_compiler.hpp"

using namespace HXSL;

int main()
{
	SetLocale("en_US");

	AssemblyCollection collection;
	HXSLCompiler::Compile({ "library.txt" }, "library.module", collection);

	//collection.LoadAssemblyFromFile("library.module");

	//HXSLCompiler::Compile({ "shader.txt" , "shader2.txt" }, "shader.module", collection);

	return 0;
}