#include "symbol_collector.hpp"

namespace HXSL
{
	inline bool SymbolCollector::Push(const TextSpan& span, SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type)
	{
		stack.push(current);
		size_t newIndex = targetAssembly->AddSymbol(span, def, metadata, current.NodeIndex);
		if (newIndex == 0)
		{
			analyzer.LogError("Redefinition of symbol '%s'", span, span.toString().c_str());
			return false;
		}

		current.NodeIndex = newIndex;
		current.Type = type;
		current.Parent = def;
		return true;
	}

	bool SymbolCollector::PushScope(const ASTNode* parent, const TextSpan& span, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type)
	{
		stack.push(current);
		size_t newIndex = targetAssembly->AddSymbolScope(span, metadata, current.NodeIndex);
		if (newIndex == 0)
		{
			analyzer.LogError("Redefinition of symbol '%s'", span, span.toString().c_str());
			return false;
		}
		current.NodeIndex = newIndex;
		current.Type = type;
		current.Parent = parent;
		return true;
	}

	inline bool SymbolCollector::PushLeaf(SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata)
	{
		auto& span = def->GetName();
		size_t newIndex = targetAssembly->AddSymbol(span, def, metadata, current.NodeIndex);
		if (newIndex == 0)
		{
			analyzer.LogError("Redefinition of symbol '%s'", span, span.toString().c_str());
			return false;
		}
		return true;
	}

	void SymbolCollector::VisitClose(ASTNode* node, size_t depth)
	{
		if (current.Parent != node) return;
		current = stack.top();
		stack.pop();
	}

	TraversalBehavior SymbolCollector::Visit(ASTNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		auto& type = node->GetType();
		switch (type)
		{
		case NodeType_Namespace:
		{
			Namespace* s = dynamic_cast<Namespace*>(node);
			auto metadata = std::make_shared<SymbolMetadata>(SymbolType_Namespace, current.Type, AccessModifier_Public, 0, s);
			Push(s->GetName(), s, metadata, SymbolScopeType_Namespace);
		}
		break;

		case NodeType_Struct:
		{
			Struct* s = dynamic_cast<Struct*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			Push(s->GetName(), s, metadata, SymbolScopeType_Struct);
		}
		break;

		case NodeType_Field:
		{
			Field* s = dynamic_cast<Field*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			PushLeaf(s, metadata);
		}
		break;

		case NodeType_FunctionOverload:
		{
			FunctionOverload* s = dynamic_cast<FunctionOverload*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			auto signature = s->BuildOverloadSignature();
			Push(TextSpan(signature), s, metadata, SymbolScopeType_Function);
		}
		break;

		case NodeType_OperatorOverload:
		{
			OperatorOverload* s = dynamic_cast<OperatorOverload*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			auto signature = s->BuildOverloadSignature();
			Push(TextSpan(signature), s, metadata, SymbolScopeType_Operator);
		}
		break;

		case NodeType_Parameter:
		{
			Parameter* s = dynamic_cast<Parameter*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, AccessModifier_Private, 0, s);
			PushLeaf(s, metadata);
		}
		break;

		case NodeType_BlockStatement:
		{
			BlockStatement* s = dynamic_cast<BlockStatement*>(node);
			auto metadata = std::make_shared<SymbolMetadata>(SymbolType_Scope, current.Type, AccessModifier_Private, 0);

			std::string temp = MakeScopeId(current.ScopeCounter++);

			PushScope(node, TextSpan(temp), metadata, SymbolScopeType_Block);
		}
		break;

		case NodeType_DeclarationStatement:
		{
			DeclarationStatement* s = dynamic_cast<DeclarationStatement*>(node);
			auto& span = s->GetName();
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, AccessModifier_Private, 0, s);
			PushLeaf(s, metadata);
		}
		break;
		}

		return TraversalBehavior_Keep;
	}
}