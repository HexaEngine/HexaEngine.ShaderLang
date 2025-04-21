#include "symbol_collector.hpp"

namespace HXSL
{
	inline bool HXSLSymbolCollector::Push(HXSLSymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, HXSLSymbolScopeType type)
	{
		auto& span = def->GetName();
		stack.push(current);
		size_t newIndex = targetAssembly->AddSymbol(def, metadata, current.NodeIndex);
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

	bool HXSLSymbolCollector::PushScope(const HXSLNode* parent, TextSpan span, std::shared_ptr<SymbolMetadata>& metadata, HXSLSymbolScopeType type)
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

	inline bool HXSLSymbolCollector::PushLeaf(HXSLSymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata)
	{
		auto& span = def->GetName();
		size_t newIndex = targetAssembly->AddSymbol(def, metadata, current.NodeIndex);
		if (newIndex == 0)
		{
			analyzer.LogError("Redefinition of symbol '%s'", span, span.toString().c_str());
			return false;
		}
		return true;
	}

	void HXSLSymbolCollector::VisitClose(HXSLNode* node, size_t depth)
	{
		if (current.Parent != node) return;
		current = stack.top();
		stack.pop();
	}

	HXSLTraversalBehavior HXSLSymbolCollector::Visit(HXSLNode*& node, size_t depth, bool deferred, EmptyDeferralContext& context)
	{
		auto& type = node->GetType();
		switch (type)
		{
		case HXSLNodeType_Namespace:
		{
			HXSLNamespace* s = dynamic_cast<HXSLNamespace*>(node);
			auto metadata = std::make_shared<SymbolMetadata>(HXSLSymbolType_Namespace, current.Type, HXSLAccessModifier_Public, 0, s);
			Push(s, metadata, HXSLSymbolScopeType_Namespace);
		}
		break;

		case HXSLNodeType_Struct:
		{
			HXSLStruct* s = dynamic_cast<HXSLStruct*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			Push(s, metadata, HXSLSymbolScopeType_Struct);
		}
		break;

		case HXSLNodeType_Field:
		{
			HXSLField* s = dynamic_cast<HXSLField*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			PushLeaf(s, metadata);
		}
		break;

		case HXSLNodeType_Function:
		{
			HXSLFunction* s = dynamic_cast<HXSLFunction*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			Push(s, metadata, HXSLSymbolScopeType_Function);
		}
		break;

		case HXSLNodeType_Parameter:
		{
			HXSLParameter* s = dynamic_cast<HXSLParameter*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, HXSLAccessModifier_Private, 0, s);
			PushLeaf(s, metadata);
		}
		break;

		case HXSLNodeType_BlockStatement:
		{
			HXSLBlockStatement* s = dynamic_cast<HXSLBlockStatement*>(node);
			auto metadata = std::make_shared<SymbolMetadata>(HXSLSymbolType_Scope, current.Type, HXSLAccessModifier_Private, 0);

			std::string temp = MakeScopeId(current.ScopeCounter++);

			PushScope(node, TextSpan(temp), metadata, HXSLSymbolScopeType_Block);
		}
		break;

		case HXSLNodeType_DeclarationStatement:
		{
			HXSLDeclarationStatement* s = dynamic_cast<HXSLDeclarationStatement*>(node);
			auto& span = s->GetName();
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, HXSLAccessModifier_Private, 0, s);
			PushLeaf(s, metadata);
		}
		break;
		}

		return HXSLTraversalBehavior_Keep;
	}
}