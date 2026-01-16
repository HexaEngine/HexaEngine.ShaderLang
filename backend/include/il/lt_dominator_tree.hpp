#ifndef LT_DOMINATOR_TREE_HPP
#define LT_DOMINATOR_TREE_HPP

#include "control_flow_graph.hpp"
#include "pch/std.hpp"

namespace HXSL
{
	namespace Backend
	{
		class LTDominatorTree
		{
			static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
			const ControlFlowGraph& cfg;
			size_t n;

			std::vector<size_t> semi, idom, ancestor, label, parent, vertex;
			std::vector<std::vector<size_t>> bucket;
			std::vector<size_t> dfsOrder;
			std::stack<size_t> walkStack;

			size_t time = 0;

		public:
			LTDominatorTree(const ControlFlowGraph& cfg) : cfg(cfg), n(cfg.size())
			{
				semi.resize(n, INVALID_INDEX);
				idom.resize(n, INVALID_INDEX);
				ancestor.resize(n, INVALID_INDEX);
				label.resize(n);
				parent.resize(n, INVALID_INDEX);
				vertex.resize(n, INVALID_INDEX);
				bucket.resize(n);
				dfsOrder.reserve(n);
			}

			void DFS(size_t start)
			{
				std::stack<size_t> walkStack;
				walkStack.push(start);

				while (!walkStack.empty())
				{
					size_t v = walkStack.top();
					walkStack.pop();

					if (semi[v] != INVALID_INDEX)
						continue;

					semi[v] = time;
					vertex[time] = v;
					label[v] = v;
					time++;

					auto& successors = cfg.nodes[v]->GetSuccessors();
					for (auto it = successors.rbegin(); it != successors.rend(); ++it)
					{
						auto w = *it;
						if (semi[w] == INVALID_INDEX)
						{
							parent[w] = v;
							walkStack.push(w);
						}
					}
				}
			}

			void Link(size_t v, size_t w)
			{
				ancestor[w] = v;
			}

			size_t Eval(size_t v)
			{
				if (ancestor[v] == INVALID_INDEX) return label[v];
				Compress(v);
				return label[v];
			}

			void Compress(size_t v)
			{
				if (ancestor[ancestor[v]] != INVALID_INDEX)
				{
					Compress(ancestor[v]);
					if (semi[label[ancestor[v]]] < semi[label[v]])
					{
						label[v] = label[ancestor[v]];
					}
					ancestor[v] = ancestor[ancestor[v]];
				}
			}

			std::vector<size_t> Compute(size_t start)
			{
				time = 0;
				DFS(start);

				for (int i = (int)time - 1; i >= 1; i--)
				{
					size_t w = vertex[i];
					for (auto v : cfg.nodes[w]->GetPredecessors())
					{
						size_t u = Eval(v);
						if (semi[u] < semi[w])
						{
							semi[w] = semi[u];
						}
					}

					bucket[vertex[semi[w]]].push_back(w);
					Link(parent[w], w);

					for (auto v : bucket[parent[w]])
					{
						size_t u = Eval(v);
						idom[v] = (semi[u] < semi[v]) ? u : parent[w];
					}
					bucket[parent[w]].clear();
				}

				for (size_t i = 1; i < time; i++)
				{
					size_t w = vertex[i];

					if (idom[w] != vertex[semi[w]])
					{
						idom[w] = idom[idom[w]];
					}
				}

				idom[start] = start;
				return idom;
			}

			std::vector<std::unordered_set<size_t>> ComputeDominanceFrontiers(const std::vector<size_t>& idom, const std::vector<std::vector<size_t>>& domTreeChildren)
			{
				const size_t n = cfg.nodes.size();
				std::vector<std::unordered_set<size_t>> df(n);

				std::stack<std::tuple<size_t, bool>> walkStack;

				for (size_t i = 0; i < n; i++)
				{
					if (idom[i] == i)
					{
						walkStack.push({ i , false });
						break;
					}
				}

				while (!walkStack.empty())
				{
					auto [node, close] = walkStack.top();
					walkStack.pop();

					auto& children = domTreeChildren[node];
					if (!close && !children.empty())
					{
						walkStack.push({ node,true });
						for (auto it = children.rbegin(); it != children.rend(); ++it)
						{
							walkStack.push({ *it, false });
						}
						continue;
					}

					for (size_t succ : cfg.nodes[node]->GetSuccessors())
					{
						if (idom[succ] != node)
						{
							df[node].insert(succ);
						}
					}

					for (size_t child : domTreeChildren[node])
					{
						for (size_t frontier_node : df[child])
						{
							if (idom[frontier_node] != node)
							{
								df[node].insert(frontier_node);
							}
						}
					}
				}

				return df;
			}
		};
	}
}

#endif