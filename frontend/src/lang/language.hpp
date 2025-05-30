#ifndef LANGUAGE_HPP
#define LANGUAGE_HPP

#include "keyword.hpp"
#include "operator.hpp"
#include "core/language.hpp"

namespace HXSL
{
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
}
#endif