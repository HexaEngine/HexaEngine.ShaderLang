#include "il_context.hpp"
#include "il_builder.hpp"

namespace HXSL
{
	void ILContext::Build()
	{
		ILContainer container = { allocator };
		JumpTable jumpTable = {};

		ILBuilder builder = ILBuilder(this, allocator, compilation, container, metadata, jumpTable);
		builder.Build(overload);
		jumpTable.Prepare();
		builder.Print();

		cfg.Build(container, jumpTable);
		cfg.Print();
	}
}