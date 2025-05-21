#include "ast_flattener.hpp"

namespace HXSL
{
	std::unique_ptr<LowerCompilationUnit> ASTFlattener::Flatten(CompilationUnit* compilation)
	{
		compilationUnit = std::make_unique<LowerCompilationUnit>();
		Traverse(compilation);
		return std::move(compilationUnit);
	}

	TraversalBehavior ASTFlattener::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		auto type = node->GetType();

		switch (type)
		{
		case HXSL::NodeType_Namespace:
		{
			auto ns = node->As<Namespace>();
			ns->TransferContentsTo(*compilationUnit.get());
		}
		break;
		case HXSL::NodeType_Struct:
		{
			auto str = node->As<Struct>();
			str->TransferContentsTo(*compilationUnit.get());
		}
		break;
		case HXSL::NodeType_Class:
		{
			auto cls = node->As<Class>();
			cls->TransferContentsTo(*compilationUnit.get());
		}
		break;
		}

		return TraversalBehavior_Keep;
	}
}