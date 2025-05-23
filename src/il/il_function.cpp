#include "il_function.hpp"
#include "il_builder.hpp"

namespace HXSL
{
	void ILFunction::Build()
	{
		ILContainer container = {};
		JumpTable jumpTable = {};

		ILBuilder builder = ILBuilder(container, metadata, jumpTable);
		builder.Build(overload);

		cfg.Build(container, jumpTable);
	}
}