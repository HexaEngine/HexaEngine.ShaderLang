#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "keyword.hpp"
#include "operator.hpp"

namespace HXSL
{
	enum AccessModifier : char
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

	enum ParameterFlags : char
	{
		ParameterFlags_None = -1,
		ParameterFlags_In = 0,
		ParameterFlags_Out = 1,
		ParameterFlags_InOut = 2,
		ParameterFlags_Uniform = 4,
		ParameterFlags_Precise = 8,
	};

	enum InterpolationModifier : char
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

	enum OperatorFlags : char
	{
		OperatorFlags_None = 0,
		OperatorFlags_Explicit = 1,
		OperatorFlags_Implicit = 2,
		OperatorFlags_Intrinsic = 4,
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

#pragma warning(push)
#pragma warning(disable : 26827) // FunctionFlags_All will be extended in the future.

	enum FunctionFlags : char
	{
		FunctionFlags_None = 0,
		FunctionFlags_Inline = 1,
		FunctionFlags_All = FunctionFlags_Inline,
	};

	static FunctionFlags operator|(FunctionFlags lhs, FunctionFlags rhs)
	{
		return static_cast<FunctionFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

#pragma warning(pop)

	static FunctionFlags& operator|=(FunctionFlags& lhs, FunctionFlags rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum StorageClass : char
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

	enum PrimitiveKind : char
	{
		PrimitiveKind_Void,
		PrimitiveKind_Bool,
		PrimitiveKind_Int,
		PrimitiveKind_UInt,
		PrimitiveKind_Half,
		PrimitiveKind_Float,
		PrimitiveKind_Double,
		PrimitiveKind_Min8Float,
		PrimitiveKind_Min10Float,
		PrimitiveKind_Min16Float,
		PrimitiveKind_Min12Int,
		PrimitiveKind_Min16Int,
		PrimitiveKind_Min16UInt,
		PrimitiveKind_Int8,
		PrimitiveKind_UInt8,
		PrimitiveKind_Int16,
		PrimitiveKind_UInt16,
		PrimitiveKind_Int64,
		PrimitiveKind_UInt64,
	};

	static PrimitiveKind& operator++(PrimitiveKind& kind)
	{
		kind = static_cast<PrimitiveKind>(static_cast<int>(kind) + 1);
		return kind;
	}

	static std::string ToString(PrimitiveKind kind)
	{
		switch (kind)
		{
		case PrimitiveKind_Void: return "void";
		case PrimitiveKind_Bool: return "bool";
		case PrimitiveKind_Int: return "int";
		case PrimitiveKind_UInt: return "uint";
		case PrimitiveKind_Half: return "half";
		case PrimitiveKind_Float: return "float";
		case PrimitiveKind_Double: return "double";
		case PrimitiveKind_Min8Float: return "min8float";
		case PrimitiveKind_Min10Float: return "min10float";
		case PrimitiveKind_Min16Float: return "min16float";
		case PrimitiveKind_Min12Int: return "min12int";
		case PrimitiveKind_Min16Int: return "min16int";
		case PrimitiveKind_Min16UInt: return "min16uint";

		default:
			return "";
			break;
		}
	}

	enum PrimitiveClass : char
	{
		PrimitiveClass_Scalar = 0,
		PrimitiveClass_Vector = 1,
		PrimitiveClass_Matrix = 2,
	};
}
#endif