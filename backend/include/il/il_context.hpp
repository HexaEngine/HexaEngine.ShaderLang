#ifndef IL_CONTEXT_HPP
#define IL_CONTEXT_HPP

#include "il_metadata.hpp"
#include "control_flow_graph.hpp"
#include "utils/bump_allocator.hpp"
#include "core/module.hpp"

namespace HXSL
{
	namespace Backend
	{
		class ILContext
		{
		public:
			BumpAllocator allocator;
			Module* module;
			FunctionLayout* function;
			ILMetadata metadata;
			ControlFlowGraph cfg;

			ILContext(Module* module, FunctionLayout* function) : module(module), function(function), metadata(allocator), cfg(this)
			{
			}

			BumpAllocator& GetAllocator() { return allocator; }

			Module* GetModule() const { return module; }

			bool empty() const noexcept { return cfg.empty(); }

			ILMetadata& GetMetadata() { return metadata; }

			const ILMetadata& GetMetadata() const { return metadata; }

			ControlFlowGraph& GetCFG() { return cfg; }

			template <typename T, class... Args>
			T* Alloc(Args&&... args)
			{
				return allocator.Alloc<T>(std::forward<Args>(args)...);
			}

			Constant* MakeConstant(const Number& num) { return allocator.Alloc<Constant>(num); }
			Variable* MakeVariable(const ILVarId& varId) { return allocator.Alloc<Variable>(varId); }
			Variable* MakeVariable(const ILVariable& var) { return allocator.Alloc<Variable>(var.id); }
		};
	}
}

#endif