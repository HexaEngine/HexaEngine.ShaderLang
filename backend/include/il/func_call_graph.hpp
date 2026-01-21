#ifndef FUNC_CALL_GRAPH_HPP
#define FUNC_CALL_GRAPH_HPP

#include "scc_graph.hpp"
#include "graph_base.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	namespace Backend
	{
		struct FCGNode : public GraphNode<FCGNode>
		{
		public:
			static constexpr size_t InvalidIndex = static_cast<size_t>(-1);
		private:
			FunctionLayout* function;
			size_t index;
			size_t sccIndex = InvalidIndex;
			float inlineCost = 0;
			std::vector<size_t> dependencies;
			std::vector<size_t> dependants;

		public:
			FCGNode(FunctionLayout* function, size_t index) : function(function), index(index)
			{
			}

			void SetFunction(FunctionLayout* func) { function = func; }
			void SetIndex(size_t idx) { index = idx; }
			void SetSCCIndex(size_t sccIdx) { sccIndex = sccIdx; }
			void SetInlineCost(float cost) { inlineCost = cost; }

			FunctionLayout* GetFunction() const { return function; }

			const size_t GetIndex() const { return index; }
			const size_t GetSCCIndex() const { return sccIndex; }
			float GetInlineCost() const { return inlineCost; }

			const std::vector<size_t>& GetDependencies() const { return dependencies; }
			const std::vector<size_t>& GetDependants() const { return dependants; }

			void AddDependency(size_t depIdx) { dependencies.push_back(depIdx); }
			void AddDependant(size_t depIdx) { dependants.push_back(depIdx); }
		};

		class FuncCallGraph : public GraphPtrBase<FCGNode>
		{
			dense_map<FunctionLayout*, size_t> valueToNode;
			std::vector<std::vector<size_t>> sccs;

		public:
			static constexpr auto InvalidIndex = FCGNode::InvalidIndex;

			void Clear()
			{
				nodes.clear();
				valueToNode.clear();
				sccs.clear();
			}

			size_t GetIndex(FunctionLayout* func) const
			{
				auto it = valueToNode.find(func);
				if (it == valueToNode.end())
				{
					return InvalidIndex;
				}
				return it->second;
			}

			FCGNode* GetNode(FunctionLayout* func) const
			{
				auto it = valueToNode.find(func);
				if (it == valueToNode.end())
				{
					return nullptr;
				}
				return nodes[it->second].get();
			}

			FCGNode* AddFunction(FunctionLayout* func)
			{
				auto index = nodes.size();
				nodes.emplace_back(make_uptr<FCGNode>(func, index));
				valueToNode[func] = index;
				return nodes.back().get();
			}

			void AddCall(FunctionLayout* caller, FunctionLayout* callee)
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
				nodeCallee->AddDependant(itCaller->second);
				nodeCaller->AddDependency(itCallee->second);
			}

			void UpdateSCCs()
			{
				sccs = SCCGraph<FCGNode>::ComputeSCCs(nodes);
				for (size_t i = 0; i < sccs.size(); ++i)
				{
					for (size_t nodeIdx : sccs[i])
					{
						nodes[nodeIdx]->SetSCCIndex(i);
					}
				}
			}

			const std::vector<std::vector<size_t>>& GetSCCs() const
			{
				return sccs;
			}
		};
	}
}

#endif