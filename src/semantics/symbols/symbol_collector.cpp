#include "symbol_collector.hpp"

namespace HXSL
{
	inline bool SymbolCollector::Push(const StringSpan& span, SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type)
	{
		stack.push(current);
		SymbolHandle handle = targetAssembly->AddSymbol(span, def, metadata, current.NodeIndex);
		if (handle.invalid())
		{
			analyzer.Log(SYMBOL_REDEFINITION, def->GetSpan(), span.str());
			return false;
		}

		current.NodeIndex = handle.GetIndex();
		current.Type = type;
		current.Parent = def;
		return true;
	}

	bool SymbolCollector::PushScope(const ASTNode* parent, const StringSpan& span, std::shared_ptr<SymbolMetadata>& metadata, SymbolScopeType type)
	{
		stack.push(current);
		SymbolHandle handle = targetAssembly->AddSymbolScope(span, metadata, current.NodeIndex);
		if (handle.invalid())
		{
			analyzer.Log(SYMBOL_REDEFINITION, parent->GetSpan(), span.str());
			return false;
		}
		current.NodeIndex = handle.GetIndex();
		current.Type = type;
		current.Parent = parent;
		return true;
	}

	inline bool SymbolCollector::PushLeaf(SymbolDef* def, std::shared_ptr<SymbolMetadata>& metadata)
	{
		auto& span = def->GetName();
		SymbolHandle handle = targetAssembly->AddSymbol(span, def, metadata, current.NodeIndex);
		if (handle.invalid())
		{
			analyzer.Log(SYMBOL_REDEFINITION, def->GetSpan(), span);
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
			auto signature = s->BuildTemporaryOverloadSignature();
			Push(signature, s, metadata, SymbolScopeType_Function);
			RegisterForLatePass(node);
		}
		break;

		case NodeType_ConstructorOverload:
		{
			ConstructorOverload* s = node->As<ConstructorOverload>();
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			auto signature = s->BuildTemporaryOverloadSignature();
			Push(signature, s, metadata, SymbolScopeType_Constructor);
			RegisterForLatePass(node);
		}
		break;

		case NodeType_OperatorOverload:
		{
			OperatorOverload* s = dynamic_cast<OperatorOverload*>(node);
			auto symType = s->GetSymbolType();
			auto metadata = std::make_shared<SymbolMetadata>(symType, current.Type, s->GetAccessModifiers(), 0, s);
			auto signature = s->BuildTemporaryOverloadSignature();
			Push(signature, s, metadata, SymbolScopeType_Operator);
			RegisterForLatePass(node);
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
			PushScope(node, temp, metadata, SymbolScopeType_Block);
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

	void SymbolCollector::LateTraverse()
	{
		auto table = targetAssembly->GetMutableSymbolTable();
		for (auto node : lateNodes)
		{
			auto& type = node->GetType();
			switch (type)
			{
			case NodeType_FunctionOverload:
			{
				FunctionOverload* s = dynamic_cast<FunctionOverload*>(node);
				auto signature = s->BuildOverloadSignature();
				auto& index = s->GetSymbolHandle();
				if (!table->RenameNode(signature, index))
				{
					analyzer.Log(SYMBOL_REDEFINITION, s->GetSpan(), signature);
				}
			}
			break;

			case NodeType_ConstructorOverload:
			{
				ConstructorOverload* s = node->As<ConstructorOverload>();
				auto signature = s->BuildOverloadSignature();
				auto& index = s->GetSymbolHandle();
				if (!table->RenameNode(signature, index))
				{
					analyzer.Log(SYMBOL_REDEFINITION, s->GetSpan(), signature);
				}
			}
			break;

			case NodeType_OperatorOverload:
			{
				OperatorOverload* s = dynamic_cast<OperatorOverload*>(node);
				auto signature = s->BuildOverloadSignature();
				auto& index = s->GetSymbolHandle();
				if (!table->RenameNode(signature, index))
				{
					analyzer.Log(SYMBOL_REDEFINITION, s->GetSpan(), signature);
				}
			}
			break;
			}
		}

		lateNodes.clear();
	}
}