#include "hxls_compiler.hpp"
#include "preprocessing/evaluator.hpp"

using namespace HXSL;

int main()
{
	SetLocale("en_US");

	AssemblyCollection collection;
	Compiler* compiler = new Compiler();
	compiler->Compile({ "example/library.txt" }, "library.module", collection);
	delete compiler;
	//collection.LoadAssemblyFromFile("library.module");

	//HXSLCompiler::Compile({ "shader.txt" , "shader2.txt" }, "shader.module", collection);

	return 0;
}