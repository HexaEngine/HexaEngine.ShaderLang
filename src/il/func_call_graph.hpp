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

		std::vector<std::vector<size_t>> ComputeSCCs() const
		{
			return SCCGraph<FCGNode<T>>::ComputeSCCs(nodes);
		}
	};
}

#endif