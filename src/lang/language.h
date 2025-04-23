#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "keyword.h"
#include "operator.h"

namespace HXSL
{
	enum AccessModifier
	{
		AccessModifier_None = 0,
		AccessModifier_Private = 1,
		AccessModifier_Protected = 2,
		AccessModifier_Internal = 4,
		AccessModifier_ProtectedInternal = AccessModifier_Protected | AccessModifier_Internal,
		AccessModifier_Public = 8,
		AccessModifier_All = AccessModifier_Private | AccessModifier_Protected | AccessModifier_Internal | AccessModifier_Public
	};

	static AccessModifier operator|(AccessModifier lhs, AccessModifier rhs)
	{
		return static_cast<AccessModifier>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	static AccessModifier& operator|=(AccessModifier& lhs, AccessModifier rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum ParameterFlags
	{
		ParameterFlags_In = 0,
		ParameterFlags_Out = 1,
		ParameterFlags_InOut = 2,
		ParameterFlags_Uniform = 4,
		ParameterFlags_Precise = 8,
	};

	enum InterpolationModifier
	{
		InterpolationModifier_None = 0,
		InterpolationModifier_Linear = 1,
		InterpolationModifier_Centroid = 2,
		InterpolationModifier_NoInterpolation = 4,
		InterpolationModifier_NoPerspecitve = 8,
		InterpolationModifier_Sample = 16,
		InterpolationModifier_All = InterpolationModifier_Linear | InterpolationModifier_Centroid | InterpolationModifier_NoInterpolation | InterpolationModifier_NoPerspecitve | InterpolationModifier_Sample,
	};

	static InterpolationModifier operator|(InterpolationModifier lhs, InterpolationModifier rhs)
	{
		return static_cast<InterpolationModifier>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	static InterpolationModifier& operator|=(InterpolationModifier& lhs, InterpolationModifier rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum OperatorFlags
	{
		OperatorFlags_None = 0,
		OperatorFlags_Explicit = 1,
		OperatorFlags_Implicit = 2,
	};

	static OperatorFlags operator|(OperatorFlags lhs, OperatorFlags rhs)
	{
		return static_cast<OperatorFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	static OperatorFlags& operator|=(OperatorFlags& lhs, OperatorFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum FunctionFlags
	{
		FunctionFlags_None = 0,
		FunctionFlags_Inline = 1,
		FunctionFlags_All = FunctionFlags_Inline,
	};

	static FunctionFlags operator|(FunctionFlags lhs, FunctionFlags rhs)
	{
		return static_cast<FunctionFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	static FunctionFlags& operator|=(FunctionFlags& lhs, FunctionFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum StorageClass
	{
		StorageClass_None = 0,
		StorageClass_Const = 1,
		StorageClass_Static = 2,
		StorageClass_Precise = 4,
		StorageClass_Shared = 8,
		StorageClass_GroupShared = 16,
		StorageClass_Uniform = 32,
		StorageClass_Volatile = 64,
		StorageClass_All = StorageClass_Const | StorageClass_Static | StorageClass_Precise | StorageClass_Shared | StorageClass_GroupShared | StorageClass_Uniform | StorageClass_Volatile,
	};

	static StorageClass operator|(StorageClass lhs, StorageClass rhs)
	{
		return static_cast<StorageClass>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	static StorageClass& operator|=(StorageClass& lhs, StorageClass rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}
}
#endif