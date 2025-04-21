#ifndef NODE_VISITOR_HPP
#define NODE_VISITOR_HPP

#include "nodes.hpp"

namespace HXSL
{
	enum HXSLTraversalBehavior
	{
		HXSLTraversalBehavior_Break,
		HXSLTraversalBehavior_Skip,
		HXSLTraversalBehavior_Keep,
		HXSLTraversalBehavior_Defer,
		HXSLTraversalBehavior_AnalyzerSkip,
	};

	struct EmptyDeferralContext
	{
	};

	template<typename DeferralContext>
	class HXSLVisitor
	{
	protected:

		virtual HXSLTraversalBehavior Visit(HXSLNode*& node, size_t depth, bool deferred, DeferralContext& context) = 0;
		virtual void VisitClose(HXSLNode* node, size_t depth)
		{
		}
	public:

		using VisitFnType = std::function<HXSLTraversalBehavior(HXSLNode*& node, size_t depth, bool deferred, DeferralContext& context)>;
		using VisitCloseFnType = std::function<void(HXSLNode* node, size_t depth)>;

		void Traverse(HXSLNode* node, VisitFnType visit, VisitCloseFnType visitClose)
		{
			std::deque<std::tuple<HXSLNode*, size_t, DeferralContext>> deferredQueue;
			std::stack<std::tuple<HXSLNode*, size_t, bool>> nodeStack;
			nodeStack.push(std::make_tuple(node, 0, false));

			while (!nodeStack.empty())
			{
				auto [currentNode, depth, closing] = nodeStack.top();
				nodeStack.pop();

				if (closing)
				{
					if (visitClose)
					{
						visitClose(currentNode, depth);
					}
				}
				else
				{
					DeferralContext context;
					HXSLTraversalBehavior result = visit(currentNode, depth, false, context);

					if (result == HXSLTraversalBehavior_Break)
					{
						break;
					}
					else if (result == HXSLTraversalBehavior_Skip)
					{
						continue;
					}
					else if (result == HXSLTraversalBehavior_Defer)
					{
						deferredQueue.push_back(std::make_tuple(currentNode, depth, context));
					}
					else
					{
						nodeStack.push(std::make_tuple(currentNode, depth, true));
					}

					auto& children = currentNode->GetChildren();
					for (size_t i = children.size() - 1; i != static_cast<size_t>(-1); --i)
					{
						nodeStack.push(std::make_tuple(children[i], depth + 1, false));
					}
				}
			}

			while (!deferredQueue.empty())
			{
				auto [currentNode, depth, context] = deferredQueue.front();
				deferredQueue.pop_front();
				HXSLTraversalBehavior result = visit(currentNode, depth, true, context);
				if (result == HXSLTraversalBehavior_Break)
				{
					break;
				}
				else if (result == HXSLTraversalBehavior_Defer)
				{
					deferredQueue.push_back(std::make_tuple(currentNode, depth, context));
				}
			}
		}

		virtual void Traverse(HXSLNode* node)
		{
			Traverse(node, std::bind(&HXSLVisitor::Visit, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), std::bind(&HXSLVisitor::VisitClose, this, std::placeholders::_1, std::placeholders::_2));
		}
	};
}
#endif