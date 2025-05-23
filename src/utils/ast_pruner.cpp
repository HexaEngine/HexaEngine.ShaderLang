#include "ast_pruner.hpp"

namespace HXSL
{
	void ASTPruner::Prune(LowerCompilationUnit* compilation)
	{
		compilationUnit = compilation;
		for (auto& func : compilation->GetFunctions())
		{
			Traverse(func->GetBody().get());
			func->SetBody(nullptr);
		}
	}

	TraversalBehavior ASTPruner::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		if (auto symbolDef = dynamic_cast<SymbolDef*>(node))
		{
			auto parent = node->GetParent();
			auto type = parent->GetType();
			switch (type)
			{
			case NodeType_BlockStatement:
			{
				auto block = parent->As<BlockStatement>();
				for (auto& statement : block->GetStatementsMut())
				{
					if (statement.get() == node)
					{
						compilationUnit->AddMiscDef(UNIQUE_PTR_CAST_AST_DYN(statement, SymbolDef));
						break;
					}
				}
			}
			break;
			case NodeType_ForStatement:
			{
				auto forStatement = parent->As<ForStatement>();
				compilationUnit->AddMiscDef(UNIQUE_PTR_CAST_AST_DYN(forStatement->GetInitMut(), SymbolDef));
			}
			break;
			default:
				HXSL_ASSERT(false, "Unhandled node type in AST pruner.");
				break;
			}
		}
		return TraversalBehavior_Keep;
	}
}