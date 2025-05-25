#ifndef LIVENESS_ANALYZER_HPP
#define LIVENESS_ANALYZER_HPP

#include "il_optimizer_pass.hpp"

namespace HXSL
{
	class LivenessAnalyzer : CFGVisitor<EmptyCFGContext>
	{
	};
}

#endif