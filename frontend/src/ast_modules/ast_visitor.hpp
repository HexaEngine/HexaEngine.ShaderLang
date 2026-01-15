#ifndef AST_VISITOR_HPP
#define AST_VISITOR_HPP

#include "pch/ast.hpp"

namespace HXSL
{
	enum TraversalBehavior
	{
		TraversalBehavior_Break,
		TraversalBehavior_Skip,
		TraversalBehavior_Keep,
		TraversalBehavior_Defer,
		TraversalBehavior_DeferSubTree,
		TraversalBehavior_AnalyzerSkip, // special value for static analyzers
	};

	struct EmptyDeferralContext
	{
	};

	template<typename DeferralContext = EmptyDeferralContext>
	class ASTVisitor
	{
	private:
		struct StackContext
		{
			ASTNode* node;
			size_t depth;
			DeferralContext context;
			bool deferred;
			bool closing;
		};
		
		struct VisitChildContext
		{
			std::vector<ASTNode*>& children;
		};

		static void VisitChild(ASTNode*& child, void* userdata)
		{
			VisitChildContext* ctx = reinterpret_cast<VisitChildContext*>(userdata);
			ctx->children.push_back(child);
		}

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
			std::stack<StackContext> nodeStack;
			std::vector<ASTNode*> children;
			nodeStack.push(StackContext(node, 0, DeferralContext(), false, false));
			do
			{
				while (!nodeStack.empty())
				{
					auto ctx = std::move(nodeStack.top());
					auto currentNode = ctx.node;
					auto depth = ctx.depth;
					auto context = std::move(ctx.context);
					auto deferred = ctx.deferred;
					auto closing = ctx.closing;
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
							nodeStack.push(StackContext(currentNode, depth, context, deferred, true));
						}

						VisitChildContext visitCtx = VisitChildContext(children);
						currentNode->ForEachChild2(VisitChild, &visitCtx);
						for (size_t i = children.size() - 1; i != static_cast<size_t>(-1); --i)
						{
							nodeStack.push(StackContext(children[i], depth + 1, context, deferred, false));
						}
						children.clear();
					}
				}

				while (!deferredQueue.empty())
				{
					auto [currentNode, depth, context, subTree] = deferredQueue.front();
					deferredQueue.pop_front();
					if (subTree)
					{
						nodeStack.push(StackContext(currentNode, depth, std::move(context), true, false));
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
			Traverse(node, std::bind(&ASTVisitor::Visit, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), std::bind(&ASTVisitor::VisitClose, this, std::placeholders::_1, std::placeholders::_2));
		}
	};
}
#endif