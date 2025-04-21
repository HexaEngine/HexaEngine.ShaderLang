#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "keyword.h"
#include "operator.h"

namespace HXSL
{
	enum HXSLAccessModifier
	{
		HXSLAccessModifier_Private,
		HXSLAccessModifier_Protected,
		HXSLAccessModifier_ProtectedInternal,
		HXSLAccessModifier_Internal,
		HXSLAccessModifier_Public,
	};

	enum HXSLParameterFlags
	{
		HXSLParameterFlags_None = 0,
		HXSLParameterFlags_Out = 1,
		HXSLParameterFlags_In = 2,
		HXSLParameterFlags_InOut = HXSLParameterFlags_In | HXSLParameterFlags_Out,
		HXSLParameterFlags_Uniform = 4
	};

	enum HXSLFunctionFlags
	{
		HXSLFunctionFlags_None = 0,
		HXSLFunctionFlags_Inline = 1,
	};

	enum HXSLFieldFlags
	{
		HXSLFieldFlags_None = 0,
		HXSLFieldFlags_Static = 1,
		HXSLFieldFlags_Nointerpolation = 2,
		HXSLFieldFlags_Shared = 4,
		HXSLFieldFlags_GroupShared = 8,
		HXSLFieldFlags_Uniform = 16,
		HXSLFieldFlags_Volatile = 32,
	};
}
#endif