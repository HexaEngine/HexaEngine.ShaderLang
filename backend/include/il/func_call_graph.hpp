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
			FunctionLayout* function;
			size_t index;
			std::vector<size_t> dependencies;
			std::vector<size_t> dependants;

			FCGNode(FunctionLayout* function, const size_t& index) : function(function), index(index)
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

		class FuncCallGraph : public GraphPtrBase<FCGNode>
		{
			dense_map<FunctionLayout*, size_t> valueToNode;
			std::vector<std::vector<size_t>> sccs;

		public:
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
					return std::string::npos;
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

			void AddFunction(FunctionLayout* func)
			{
				auto index = nodes.size();
				nodes.emplace_back(make_uptr<FCGNode>(func, index));
				valueToNode[func] = index;
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
				nodeCallee->dependants.push_back(itCaller->second);
				nodeCaller->dependencies.push_back(itCallee->second);
			}

			void UpdateSCCs()
			{
				sccs = SCCGraph<FCGNode>::ComputeSCCs(nodes);
			}

			const std::vector<std::vector<size_t>>& GetSCCs() const
			{
				return sccs;
			}
		};
	}
}

#endif