#ifndef GRAPH_BASE_HPP
#define GRAPH_BASE_HPP

#include "pch/std.hpp"

namespace HXSL
{
	namespace Backend
	{
		template<class Derived>
		class GraphNode
		{
		public:
			const std::vector<size_t>& GetDependencies() const
			{
				return static_cast<const Derived*>(this)->GetDependencies();
			}

			const std::vector<size_t>& GetDependants() const
			{
				return static_cast<const Derived*>(this)->GetDependants();
			}
		};

		template <typename T>
		concept IsGraphNode = std::is_base_of_v<GraphNode<T>, T>;

		template <IsGraphNode TNode>
		class GraphBase
		{
		protected:
			std::vector<TNode> nodes;

		public:
			size_t size() const noexcept { return nodes.size(); }
			bool empty() const noexcept { return nodes.empty(); }

			const std::vector<TNode>& GetNodes() const noexcept { return nodes; }
			TNode& GetNode(size_t index) { return nodes[index]; }
			const TNode& GetNode(size_t index) const { return nodes[index]; }
		};
	}
}

#endif