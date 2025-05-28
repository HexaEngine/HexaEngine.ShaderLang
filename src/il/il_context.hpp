#ifndef IL_CONTEXT_HPP
#define IL_CONTEXT_HPP

#include "il_metadata.hpp"
#include "control_flow_graph.hpp"
#include "utils/bump_allocator.hpp"

namespace HXSL
{
	class ILContext
	{
	public:
		BumpAllocator allocator;
		LowerCompilationUnit* compilation;
		FunctionOverload* overload;
		ILMetadata metadata;
		ControlFlowGraph cfg;

		ILContext(LowerCompilationUnit* compilation, FunctionOverload* overload) : compilation(compilation), overload(overload), cfg(this)
		{
		}

		ILMetadata& GetMetadata() { return metadata; }

		ControlFlowGraph& GetCFG() { return cfg; }

		void Build();

		template <typename T, class... Args>
		T* Alloc(Args&&... args)
		{
			return allocator.Alloc<T>(std::forward<Args>(args)...);
		}

		Constant* MakeConstant(const Number& num) { return allocator.Alloc<Constant>(num); }
		Variable* MakeVariable(const ILVarId& varId) { return allocator.Alloc<Variable>(varId); }
		Variable* MakeVariable(const ILVariable& var) { return allocator.Alloc<Variable>(var.id); }
		Phi* MakePhi(const ILPhiId& phiId) { return allocator.Alloc<Phi>(phiId); }
	};
}

#endif