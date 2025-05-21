#ifndef FUNC_CALL_GRAPH_HPP
#define FUNC_CALL_GRAPH_HPP

#include "scc_graph.hpp"
#include "graph_base.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	template <typename T>
	struct FCGNode : public GraphNode<FCGNode<T>>
	{
		T value;
		size_t index;
		std::vector<size_t> dependencies;
		std::vector<size_t> dependants;

		FCGNode(const T& value, const size_t& index) : value(value), index(index)
		{
		}

		const std::vector<size_t>& GetDependencies() const
		{
			return dependencies;
		}

		const std::vector<size_t>& GetDependants() const
		{
			return dependants;
		}
	};

	template <typename T>
	class FuncCallGraph
	{
		std::vector<FCGNode<T>> nodes;
		std::unordered_map<T, size_t> valueToNode;

	public:
		void AddFunction(const T& func)
		{
			auto index = nodes.size();
			FCGNode node = FCGNode(func, index);
			nodes.push_back(node);
			valueToNode.insert({ func, index });
		}

		void AddCall(const T& caller, const T& callee)
		{
			auto itCallee = valueToNode.find(callee);
			if (itCallee == valueToNode.end())
			{
				throw std::runtime_error("Callee couldn't be found");
			}
			auto itCaller = valueToNode.find(caller);
			if (itCaller == valueToNode.end())
			{
				throw std::runtime_error("Caller couldn't be found");
			}
			auto& nodeCallee = nodes[itCallee->second];
			auto& nodeCaller = nodes[itCaller->second];
			nodeCallee.dependants.push_back(itCaller->second);
			nodeCaller.dependencies.push_back(itCallee->second);
		}

		std::vector<std::vector<size_t>> ComputeSCCs()
		{
			SCCGraph<FCGNode<T>> graph;
			return graph.ComputeSCCs(nodes);
			size_t N = nodes.size();
			std::vector<size_t> index(N, -1);
			std::vector<size_t> boundary(N, -1);
			std::vector<bool> onStack(N, false);
			std::vector<std::vector<size_t>> sccs;

			std::stack<size_t> S;
			std::stack<size_t> P;

			size_t currentIndex = 0;

			struct Frame
			{
				size_t v;
				size_t dep_i;
				bool finished;
				Frame(size_t v) : v(v), dep_i(0), finished(false) {}
			};

			std::stack<Frame> stackFrames;

			for (size_t start = 0; start < N; ++start)
			{
				if (index[start] != -1)
					continue;

				stackFrames.push(start);

				while (!stackFrames.empty())
				{
					Frame& frame = stackFrames.top();
					size_t v = frame.v;

					if (index[v] == -1)
					{
						index[v] = currentIndex++;
						boundary[v] = S.size();
						S.push(v);
						P.push(boundary[v]);
						onStack[v] = true;
					}

					if (frame.dep_i < nodes[v].dependencies.size())
					{
						size_t w = nodes[v].dependencies[frame.dep_i];
						frame.dep_i++;

						if (index[w] == -1)
						{
							stackFrames.push(w);
							continue;
						}
						else if (onStack[w])
						{
							while (!P.empty() && P.top() > index[w])
								P.pop();
						}
					}
					else
					{
						if (!P.empty() && P.top() == boundary[v])
						{
							P.pop();

							std::vector<size_t> scc;
							size_t w;
							do
							{
								w = S.top();
								S.pop();
								onStack[w] = false;
								scc.push_back(w);
							} while (w != v);

							sccs.push_back(std::move(scc));
						}
						stackFrames.pop();
					}
				}
			}

			return sccs;
		}
	};
}

#endif