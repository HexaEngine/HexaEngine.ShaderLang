#include "il/il_context.hpp"
#include "il/il_code_blob.hpp"

namespace HXSL
{
	namespace Backend
	{
		ILContext::ILContext(Module* module, FunctionLayout* function, ILCodeBlob& blob) : module(module), function(function), metadata(allocator), cfg(this), loopTree(cfg)
		{
			SetMetadata(blob.GetMetadata());
			auto& cfg = GetCFG();
			cfg.Build(blob.GetInstructions(), blob.GetJumpTable());
		}
	}
}