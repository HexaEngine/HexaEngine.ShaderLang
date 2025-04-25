#ifndef NODE_VISITOR_HPP
#define NODE_VISITOR_HPP

#include "ast.hpp"

namespace HXSL
{
	enum TraversalBehavior
	{
		TraversalBehavior_Break,
		TraversalBehavior_Skip,
		TraversalBehavior_Keep,
		TraversalBehavior_Defer,
		TraversalBehavior_DeferSubTree,
		TraversalBehavior_AnalyzerSkip,
	};

	struct EmptyDeferralContext
	{
	};

	template<typename DeferralContext>
	class Visitor
	{
	protected:

		virtual TraversalBehavior Visit(ASTNode*& node, size_t depth, bool deferred, DeferralContext& context) = 0;
		virtual void VisitClose(ASTNode* node, size_t depth)
		{
		}
	public:

		using VisitFnType = std::function<TraversalBehavior(ASTNode*& node, size_t depth, bool deferred, DeferralContext& context)>;
		using VisitCloseFnType = std::function<void(ASTNode* node, size_t depth)>;

		void Traverse(ASTNode* node, VisitFnType visit, VisitCloseFnType visitClose)
		{
			std::deque<std::tuple<ASTNode*, size_t, DeferralContext, bool>> deferredQueue;
			std::stack<std::tuple<ASTNode*, size_t, DeferralContext, bool, bool>> nodeStack;
			nodeStack.push(std::make_tuple(node, 0, DeferralContext(), false, false));
			do
			{
				while (!nodeStack.empty())
				{
					auto [currentNode, depth, context, deferred, closing] = nodeStack.top();
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
						TraversalBehavior result = visit(currentNode, depth, deferred, context);

						if (result == TraversalBehavior_Break)
						{
							break;
						}
						else if (result == TraversalBehavior_Skip)
						{
							continue;
						}
						else if (result == TraversalBehavior_Defer)
						{
							deferredQueue.push_back(std::make_tuple(currentNode, depth, context, false));
						}
						else if (result == TraversalBehavior_DeferSubTree)
						{
							deferredQueue.push_back(std::make_tuple(currentNode, depth, std::move(context), true));
							continue;
						}
						else
						{
							nodeStack.push(std::make_tuple(currentNode, depth, context, deferred, true));
						}

						auto& children = currentNode->GetChildren();
						for (size_t i = children.size() - 1; i != static_cast<size_t>(-1); --i)
						{
							nodeStack.push(std::make_tuple(children[i], depth + 1, context, deferred, false));
						}
					}
				}

				while (!deferredQueue.empty())
				{
					auto [currentNode, depth, context, subTree] = deferredQueue.front();
					deferredQueue.pop_front();
					if (subTree)
					{
						nodeStack.push(std::make_tuple(currentNode, depth, std::move(context), true, false));
						continue;
					}
					TraversalBehavior result = visit(currentNode, depth, true, context);
					if (result == TraversalBehavior_Break)
					{
						break;
					}
					else if (result == TraversalBehavior_Defer)
					{
						deferredQueue.push_back(std::make_tuple(currentNode, depth, std::move(context), false));
					}
				}
			} while (!nodeStack.empty());
		}

		virtual void Traverse(ASTNode* node)
		{
			Traverse(node, std::bind(&Visitor::Visit, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), std::bind(&Visitor::VisitClose, this, std::placeholders::_1, std::placeholders::_2));
		}
	};
}
#endif