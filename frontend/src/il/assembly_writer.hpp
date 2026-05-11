#pragma once

#include "assembly_format.hpp"

namespace HXSL
{
	class AssemblyWriter
	{
		ObjPtr<Stream> stream;
	public:
		AssemblyWriter(const ObjPtr<Stream>& stream) : stream(stream) {}

		void Write(Assembly* assembly);
	};
}