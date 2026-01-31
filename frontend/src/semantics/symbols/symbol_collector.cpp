#include "symbol_collector.hpp"

namespace HXSL
{
	inline bool SymbolCollector::Push(const StringSpan& span, SymbolDef* def, const ObjPtr<SymbolMetadata>& metadata, SymbolScopeType type)
	{
		stack.push(current);
		SymbolHandle handle = targetAssembly->AddSymbol(span, def, metadata, current.Node);
		if (handle.invalid())
		{
			analyzer.Log(SYMBOL_REDEFINITION, def->GetSpan(), span.str());
			return false;
		}

		current.Node = handle.GetNode();
		current.Type = type;
		current.Parent = def;
		return true;
	}

	bool SymbolCollector::PushScope(const ASTNode* parent, const StringSpan& span, const ObjPtr<SymbolMetadata>& metadata, SymbolScopeType type)
	{
		stack.push(current);
		SymbolHandle handle = targetAssembly->AddSymbolScope(span, metadata, current.Node);
		if (handle.invalid())
		{
			analyzer.Log(SYMBOL_REDEFINITION, parent->GetSpan(), span.str());
			return false;
		}
		current.Node = handle.GetNode();
		current.Type = type;
		current.Parent = parent;
		return true;
	}

	inline bool SymbolCollector::PushLeaf(SymbolDef* def, const ObjPtr<SymbolMetadata>& metadata)
	{
		auto& span = def->GetName();
		SymbolHandle handle = targetAssembly->AddSymbol(span, def, metadata, current.Node);
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
		auto type = node->GetType();
		switch (type)
		{
		case NodeType_Namespace:
		{
			Namespace* s = cast<Namespace>(node);
			auto metadata = SymbolMetadata::Create(s);
			Push(s->GetName(), s, metadata, SymbolScopeType_Namespace);
		}
		break;

		case NodeType_Struct:
		{
			Struct* s = cast<Struct>(node);
			auto symType = s->GetSymbolType();
			auto metadata = SymbolMetadata::Create(s);
			Push(s->GetName(), s, metadata, SymbolScopeType_Struct);
		}
		break;

		case NodeType_Field:
		{
			Field* s = cast<Field>(node);
			auto symType = s->GetSymbolType();
			auto metadata = SymbolMetadata::Create(s);
			PushLeaf(s, metadata);
		}
		break;

		case NodeType_FunctionOverload:
		{
			FunctionOverload* s = cast<FunctionOverload>(node);
			auto name = s->GetName();
			auto nodeChild = current.Node->GetChild(name);
			if (!nodeChild)
			{
				auto table = targetAssembly->GetMutableSymbolTable();
				nodeChild = table->Insert(name, {}, current.Node);
			}

			stack.push(current);
			auto symType = s->GetSymbolType();
			auto metadata = SymbolMetadata::Create(s);
			auto signature = s->BuildTemporaryOverloadSignature();
			SymbolHandle handle = targetAssembly->AddSymbol(signature, s, metadata, nodeChild);
			if (handle.invalid())
			{
				analyzer.Log(SYMBOL_REDEFINITION, s->GetSpan(), signature);
			}
			else
			{
				current.Node = handle.GetNode();
				current.Type = SymbolScopeType_Function;
				current.Parent = s;
			}
		
			RegisterForLatePass(node);
		}
		break;

		case NodeType_ConstructorOverload:
		{
			ConstructorOverload* s = cast<ConstructorOverload>(node);
			auto symType = s->GetSymbolType();
			auto metadata = SymbolMetadata::Create(s);
			auto signature = s->BuildTemporaryOverloadSignature();
			Push(signature, s, metadata, SymbolScopeType_Constructor);
			RegisterForLatePass(node);
		}
		break;

		case NodeType_OperatorOverload:
		{
			OperatorOverload* s = cast<OperatorOverload>(node);
			auto symType = s->GetSymbolType();
			auto metadata = SymbolMetadata::Create(s);
			auto signature = s->BuildTemporaryOverloadSignature();
			Push(signature, s, metadata, SymbolScopeType_Operator);
			RegisterForLatePass(node);
		}
		break;

		case NodeType_Parameter:
		{
			Parameter* s = cast<Parameter>(node);
			auto symType = s->GetSymbolType();
			auto metadata = SymbolMetadata::Create(s);
			PushLeaf(s, metadata);
		}
		break;

		case NodeType_BlockStatement:
		{
			BlockStatement* s = cast<BlockStatement>(node);
			auto metadata = SymbolMetadata::Create(nullptr);
			std::string temp = MakeScopeId(current.ScopeCounter++);
			PushScope(node, temp, metadata, SymbolScopeType_Block);
		}
		break;

		case NodeType_DeclarationStatement:
		{
			DeclarationStatement* s = cast<DeclarationStatement>(node);
			auto& span = s->GetName();
			auto symType = s->GetSymbolType();
			auto metadata = SymbolMetadata::Create(s);
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
			auto type = node->GetType();
			switch (type)
			{
			case NodeType_FunctionOverload:
			{
				FunctionOverload* s = cast<FunctionOverload>(node);
				auto signature = s->BuildOverloadSignature();
				auto& index = s->GetSymbolHandle();
				if (!table->RenameNode(signature, index))
				{
					analyzer.Log(SYMBOL_REDEFINITION, s->GetSpan(), signature);
				}
				s->UpdateFQN();
			}
			break;

			case NodeType_ConstructorOverload:
			{
				ConstructorOverload* s = cast<ConstructorOverload>(node);
				auto signature = s->BuildOverloadSignature();
				auto& index = s->GetSymbolHandle();
				if (!table->RenameNode(signature, index))
				{
					analyzer.Log(SYMBOL_REDEFINITION, s->GetSpan(), signature);
				}
				s->UpdateFQN();
			}
			break;

			case NodeType_OperatorOverload:
			{
				OperatorOverload* s = cast<OperatorOverload>(node);
				auto signature = s->BuildOverloadSignature();
				auto& index = s->GetSymbolHandle();
				if (!table->RenameNode(signature, index))
				{
					analyzer.Log(SYMBOL_REDEFINITION, s->GetSpan(), signature);
				}
				s->UpdateFQN();
			}
			break;
			}
		}

		lateNodes.clear();
	}
}