#include "utils/co_trampoline.hpp"

namespace HEXA_UTILS_NAMESPACE
{
	bool TrampolineTaskScheduler::pump()
	{
		if (stack.empty() && !queue.empty())
			stack.push_back({ 0 });

		while (!stack.empty())
		{
			auto& current = stack.back();
			if (current.start >= queue.size())
			{
				stack.pop_back();
				continue;
			}

			auto handle = std::move(queue[current.start]);

			if (!handle)
			{
				queue.resize(current.start);
				stack.pop_back();
				continue;
			}

			auto next = queue.size();

			if (!handle->IsCompleted())
			{
				handle->Execute();
			}

			if (!handle->IsCompleted())
			{
				queue[current.start] = std::move(handle);
				stack.push_back({ next });
				return false;
			}
			else
			{
				++current.start;
			}
		}

		return true;
	}
}