#ifndef SCC_GRAPH_HPP
#define SCC_GRAPH_HPP

#include "pch/std.hpp"

namespace HXSL
{
	template <class T>
	class SCCGraph
	{
	public:
		std::vector<std::vector<size_t>> ComputeSCCs(const std::vector<T>& nodes)
		{
			std::vector<std::vector<size_t>> sccs;

			size_t N = nodes.size();
			std::vector<size_t> index(N, -1);
			std::vector<size_t> lowlink(N, 0);
			std::vector<bool> onStack(N, false);
			std::stack<size_t> S;

			size_t currentIndex = 0;

			struct Frame
			{
				size_t v;
				size_t i;
				bool returning;
			};

			std::stack<Frame> stack;

			for (size_t v = 0; v < N; ++v) {
				if (index[v] != -1) continue;

				stack.emplace(v);

				while (!stack.empty())
				{
					Frame& frame = stack.top();
					size_t current = frame.v;

					if (!frame.returning)
					{
						index[current] = lowlink[current] = currentIndex++;
						S.push(current);
						onStack[current] = true;
						frame.returning = true;
					}

					while (frame.i < nodes[current].dependencies.size())
					{
						size_t w = nodes[current].dependencies[frame.i++];

						if (index[w] == -1)
						{
							stack.emplace(w);
							break;
						}
						else if (onStack[w])
						{
							lowlink[current] = std::min(lowlink[current], index[w]);
						}
					}

					if (frame.i == nodes[current].dependencies.size())
					{
						for (size_t w : nodes[current].dependencies)
						{
							if (onStack[w])
							{
								lowlink[current] = std::min(lowlink[current], lowlink[w]);
							}
						}

						if (lowlink[current] == index[current])
						{
							std::vector<size_t> scc;
							size_t w;
							do
							{
								w = S.top(); S.pop();
								onStack[w] = false;
								scc.push_back(w);
							} while (w != current);
							sccs.push_back(std::move(scc));
						}

						stack.pop();
					}
				}
			}

			return sccs;
		}
	};
}

#endif