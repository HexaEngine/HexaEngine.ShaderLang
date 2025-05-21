#ifndef IL_CONTEXT_HPP
#define IL_CONTEXT_HPP

#include "il/il_builder.hpp"

namespace HXSL
{
	struct ILContext
	{
		ILogger* logger;
		FunctionOverload* overload;
		ILBuilder builder = {};
		bool canInline = false;

		ILContext(ILogger* logger, FunctionOverload* overload) : logger(logger), overload(overload)
		{
		}

		template<typename... Args>
		void Log(DiagnosticCode code, const TextSpan& span, Args&&... args) const
		{
			logger->LogFormattedEx(code, " (Line: {}, Column: {})", std::forward<Args>(args)..., span.line, span.column);
		}

		ILMetadata& GetMetadata() { return builder.GetMetadata(); }

		void Print() { builder.Print(); }

		void Build();

		void Fold();

		void BuildCFG();

		void UpdateState();

		void TryInline(ILContext& ctx, uint64_t funcSlot);
	};
}

#endif