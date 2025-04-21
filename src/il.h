#ifndef IL_H
#define IL_H

namespace HXSL
{
	struct ILAssemblyHeader
	{
		int Version;
	};

	enum OpCode
	{
		OpCode_Unknown,
		OpCode_StructBegin,
		OpCode_Field,
		OpCode_StructEnd,
	};
}
#endif