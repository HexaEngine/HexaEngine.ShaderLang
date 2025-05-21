#include "ast_pruner.hpp"

namespace HXSL
{
	std::unique_ptr<CompilationUnit> AstPruner::Flatten(Compilation* compilation)
	{
		auto compilationUnit = std::make_unique<CompilationUnit>();
		AstFlattener flattener(compilationUnit.get());
		flattener.Traverse(compilation);
		return std::move(compilationUnit);
	}

	TraversalBehavior AstFlattener::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		auto type = node->GetType();

		switch (type)
		{
		case HXSL::NodeType_Namespace:
		{
			auto ns = node->As<Namespace>();
			ns->TransferContentsTo(*compilationUnit);
		}
		break;
		case HXSL::NodeType_Struct:
		{
			auto str = node->As<Struct>();
			str->TransferContentsTo(*compilationUnit);
		}
		break;
		case HXSL::NodeType_Class:
		{
			auto cls = node->As<Class>();
			cls->TransferContentsTo(*compilationUnit);
		}
		break;
		}

		return TraversalBehavior_Keep;
	}
}