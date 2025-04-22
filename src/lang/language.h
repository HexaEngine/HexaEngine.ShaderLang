#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "keyword.h"
#include "operator.h"

namespace HXSL 
{
	enum AccessModifier
	{
		AccessModifier_Private,
		AccessModifier_Protected,
		AccessModifier_ProtectedInternal,
		AccessModifier_Internal,
		AccessModifier_Public,
	};

	enum ParameterFlags
	{
		ParameterFlags_None = 0,
		ParameterFlags_Out = 1,
		ParameterFlags_In = 2,
		ParameterFlags_InOut = ParameterFlags_In | ParameterFlags_Out,
		ParameterFlags_Uniform = 4
	};

	enum FunctionFlags
	{
		FunctionFlags_None = 0,
		FunctionFlags_Inline = 1,
	};

	enum FieldFlags
	{
		FieldFlags_None = 0,
		FieldFlags_Static = 1,
		FieldFlags_Nointerpolation = 2,
		FieldFlags_Shared = 4,
		FieldFlags_GroupShared = 8,
		FieldFlags_Uniform = 16,
		FieldFlags_Volatile = 32,
	};
}
#endif