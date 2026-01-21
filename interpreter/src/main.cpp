#include "hxls_compiler.hpp"
#include "core/module.hpp"
#include "pch/il.hpp"
#include "il_interpreter.hpp"

using namespace HXSL;
using namespace HXSL::Backend;

int main()
{
	SetLocale("en_US");

	auto stream = FileStream::OpenRead("modules/library.module");

	ModuleReader reader = ModuleReader(stream.get());
	auto module = reader.Read();
	stream.reset();

	auto& functions = module->GetAllFunctions();
	auto func = functions[0];
	auto codeBlob = func->GetCodeBlob();

	std::vector<Number> parameters;
	parameters.push_back(Number(10.0f));
	ILInterpreter interpreter;
	Number result = interpreter.Execute(codeBlob, parameters);

	std::cout << "Execution completed. Result: " << result.ToString() << std::endl;

	return 0;
}