#ifndef DAG_GRAPH_HPP
#define DAG_GRAPH_HPP

#include "graph_base.hpp"

namespace HXSL
{
	namespace Backend
	{
		template<typename T>
		struct DAGNode : public GraphNode<DAGNode<T>>
		{
			T value;
			std::vector<size_t> dependencies;

			DAGNode(const T& value) : value(value)
			{
			}

			DAGNode(const T&& value) : value(std::move(value))
			{
			}

			DAGNode() = default;

			const std::vector<size_t>& GetDependencies() const
			{
				return dependencies;
			}

			const std::vector<size_t>& GetDependants() const
			{
				throw std::runtime_error("DAGNode does not track dependants");
			}
		};

		template<typename T>
		class DAGGraph : protected GraphBase<DAGNode<T>>
		{

		public:
			size_t AddNode(const T& value)
			{
				auto index = this->nodes.size();
				this->nodes.push_back(DAGNode<T>(value));
				return index;
			}

			size_t EmplaceNode(T&& value)
			{
				auto index = this->nodes.size();
				this->nodes.emplace_back(DAGNode<T>(std::move(value)));
				return index;
			}

			void AddEdge(size_t from, size_t to)
			{
				this->nodes[from].dependencies.push_back(to);
			}

			void Clear()
			{
				this->nodes.clear();
			}

			std::vector<size_t> TopologicalSort() const
			{
				struct Frame
				{
					size_t index;
					bool closing;
				};
				std::stack<Frame> walkStack;

				struct State
				{
					bool visited;
					bool onStack;
				};
				std::vector<State> states(this->nodes.size(), State{false, false});
				std::vector<size_t> sortedNodes;

				for (size_t i = 0; i < this->nodes.size(); ++i)
				{
					if (states[i].visited)
						continue;

					walkStack.push(Frame{i, false});

					while (!walkStack.empty())
					{
						Frame frame = walkStack.top();
						walkStack.pop();
						auto& state = states[frame.index];

						if (frame.closing)
						{
							state.onStack = false;
							sortedNodes.push_back(frame.index);
						}
						else
						{
							if (state.visited)
								continue;

							state.visited = true;
							state.onStack = true;

							walkStack.push(Frame{frame.index, true});

					for (size_t dep : this->nodes[frame.index].dependencies)
						{
							if (states[dep].onStack)
							{
								throw std::runtime_error("Cycle detected in DAG");
							}
							if (!states[dep].visited)
							{
								walkStack.push(Frame{dep, false});
							}
						}
						}
					}
				}

				std::reverse(sortedNodes.begin(), sortedNodes.end());

				return sortedNodes;
			}
		};
	}
}

#endif