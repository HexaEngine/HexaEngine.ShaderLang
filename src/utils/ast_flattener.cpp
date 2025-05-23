#include "ast_flattener.hpp"

namespace HXSL
{
	std::unique_ptr<LowerCompilationUnit> ASTFlattener::Flatten(CompilationUnit* compilation)
	{
		compilationUnit = std::make_unique<LowerCompilationUnit>();
		Traverse(compilation);
		return std::move(compilationUnit);
	}

	void ASTFlattener::VisitClose(ASTNode* node, size_t depth)
	{
		auto type = node->GetType();

		switch (type)
		{
		case HXSL::NodeType_Namespace:
		{
			auto ns = node->As<Namespace>();
			ns->TransferContentsTo(*compilationUnit.get(), ContainerTransferFlags_All);
		}
		break;
		case HXSL::NodeType_Struct:
		{
			auto str = node->As<Struct>();
			str->TransferContentsTo(*compilationUnit.get(), ContainerTransferFlags_Default);
		}
		break;
		case HXSL::NodeType_Class:
		{
			auto cls = node->As<Class>();
			cls->TransferContentsTo(*compilationUnit.get(), ContainerTransferFlags_Default);
		}
		break;
		}
	}
}