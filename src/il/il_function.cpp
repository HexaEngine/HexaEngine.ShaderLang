#include "il_function.hpp"
#include "il_builder.hpp"

namespace HXSL
{
	void ILFunction::Build()
	{
		ILContainer container = { allocator };
		JumpTable jumpTable = {};

		ILBuilder builder = ILBuilder(compilation, container, metadata, jumpTable);
		builder.Build(overload);

		cfg.Build(container, jumpTable);
		cfg.Print();
	}
}