#ifndef SCC_GRAPH_HPP
#define SCC_GRAPH_HPP

#include "pch/std.hpp"

namespace HXSL
{
	namespace Backend
	{
		template <class T>
		class SCCGraph
		{
			static constexpr size_t InvalidIndex = static_cast<size_t>(-1);

			struct State
			{
				size_t index = InvalidIndex;
				size_t lowlink = 0;
				bool onStack = false;

				constexpr bool HasIndex() const noexcept { return index != InvalidIndex; }
			};

			struct Frame
			{
				size_t parent;
				size_t v;
				size_t i = 0;
				bool returning = false;

				Frame(size_t parent, size_t v) : parent(parent), v(v) {}
			};

		public:
			static std::vector<std::vector<size_t>> ComputeSCCs(const std::vector<uptr<T>>& nodes)
			{
				std::vector<std::vector<size_t>> sccs;

				size_t N = nodes.size();
		
				std::vector<State> state(N, State{});
				std::stack<size_t> S;

				size_t currentIndex = 0;
				std::stack<Frame> stack;

				for (size_t v = 0; v < N; ++v) 
				{
					if (state[v].HasIndex()) continue;

					stack.emplace(InvalidIndex, v);

					while (!stack.empty())
					{
						Frame& frame = stack.top();
						size_t current = frame.v;
						auto& currentState = state[current];

						if (!frame.returning)
						{
							currentState.index = currentState.lowlink = currentIndex++;
							S.push(current);
							currentState.onStack = true;
							frame.returning = true;
						}

						auto& node = nodes[current];
						auto& dependencies = node->GetDependencies();

						bool recursed = false;

						while (frame.i < dependencies.size())
						{
							size_t w = dependencies[frame.i++];
							auto& wState = state[w];

							if (!wState.HasIndex())
							{
								stack.emplace(current, w);
								recursed = true;
								break;
							}
							else if (wState.onStack)
							{
								currentState.lowlink = std::min(currentState.lowlink, wState.index);
							}
						}

						if (recursed) continue;

						if (frame.i == dependencies.size())
						{
							Frame finished = stack.top();
							auto& finishedState = state[finished.v];
							stack.pop();

							if (finished.parent != InvalidIndex)
							{
								auto& parentState = state[finished.parent];
								parentState.lowlink = std::min(parentState.lowlink, finishedState.lowlink);
							}

							if (finishedState.lowlink == finishedState.index)
							{
								std::vector<size_t> scc;
								size_t w;
								do
								{
									w = S.top(); S.pop();
									auto& wState = state[w];
									wState.onStack = false;
									scc.push_back(w);
								} while (w != finished.v);
								sccs.push_back(std::move(scc));
							}
						}
					}
				}

				return sccs;
			}
		};
	}
}

#endif