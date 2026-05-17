#include "core/module_reader.hpp"
#include "il/assembly_resolver.hpp"
#include "il_interpreter.hpp"
#include <il/assembly_loader.hpp>

using namespace HXSL;
using namespace HXSL::Backend;

int main()
{
	SetLocale("en_US");

	AssemblyLoadContext loadContext;
	auto* assembly = loadContext.LoadFromFile("modules/library.module");
	auto* entryPoint = assembly->GetEntryPoint();

	std::vector<Number> parameters;
	parameters.push_back(Number(10.0f));
	ILInterpreter interpreter;
	Number result = interpreter.Execute(entryPoint->GetCodeBlob(), parameters);

	std::cout << "Execution completed. Result: " << result.ToString() << std::endl;

	return 0;
}