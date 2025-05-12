#ifndef INTRINSICS_HPP
#define INTRINSICS_HPP

#include <unordered_set>
#include <string>
#include "nodes.hpp"

namespace HXSL
{
	class IntrinsicFunction : Function
	{
	public:
		IntrinsicFunction :
		Function(TextSpan(), nullptr, NodeType_IntrinsicFunction)
		{
		}
	};

	static const std::unordered_set<std::string> functions =
	{
		"abs",
		"acos",
		"all",
		"any",
		"asdouble",
		"asfloat",
		"asin",
		"asint",
		"asuint",
		"atan",
		"atan2",
		"ceil",
		"clamp",
		"clip",
		"cos",
		"cosh",
		"cross",
		"ddx",
		"ddx_coarse",
		"ddx_fine",
		"ddy",
		"ddy_coarse",
		"ddy_fine",
		"degrees",
		"distance",
		"dot",
		"distance",
		"exp",
		"exp2",
		"floor",
		"fmod",
		"frac",
		"fwidth",
		"ldexp",
		"length",
		"lerp",
		"lit",
		"log",
		"log10",
		"log2",
		"max",
		"min",
		"normalize",
		"pow",
		"radians",
		"reflect",
		"refract",
		"round",
		"rpc",
		"rsqrt",
		"saturate",
		"sin",
		"sinh",
		"smoothstep",
		"sqrt",
		"step",
		"tan",
		"tanh"
	};
}

#endif