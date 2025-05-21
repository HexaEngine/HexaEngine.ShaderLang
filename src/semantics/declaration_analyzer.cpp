#include "declaration_analyzer.hpp"

namespace HXSL
{
	static TraversalBehavior AnalyzeStruct(SemanticAnalyzer& analyzer, Struct* node)
	{
		std::stack<Struct*> traversalStack;
		std::unordered_set<Struct*> visited;
		std::unordered_set<Struct*> processing;

		traversalStack.push(node);

		while (!traversalStack.empty())
		{
			Struct* currentNode = traversalStack.top();

			if (processing.find(currentNode) != processing.end())
			{
				analyzer.Log(RECURSIVE_STRUCT_LAYOUT, currentNode->GetSpan());
				return TraversalBehavior_Break;
			}

			if (visited.find(currentNode) != visited.end())
			{
				traversalStack.pop();
				continue;
			}

			processing.insert(currentNode);

			bool allFieldsProcessed = true;

			for (auto& field : currentNode->GetFields())
			{
				auto decl = field->GetSymbolRef()->GetDeclaration();

				if (auto fieldStruct = dynamic_cast<Struct*>(decl))
				{
					if (visited.find(fieldStruct) == visited.end())
					{
						traversalStack.push(fieldStruct);
						allFieldsProcessed = false;
					}
					else
					{
						analyzer.Log(RECURSIVE_STRUCT_LAYOUT, currentNode->GetSpan());
						return TraversalBehavior_Break;
					}
				}
			}

			if (allFieldsProcessed)
			{
				visited.insert(currentNode);
				processing.erase(currentNode);
				traversalStack.pop();
			}
		}

		return TraversalBehavior_AnalyzerSkip;
	}

	TraversalBehavior DeclarationAnalyzer::Analyze(SemanticAnalyzer& analyzer, ASTNode* node, Compilation* compilation)
	{
		auto& type = node->GetType();

		switch (type)
		{
		case NodeType_Struct:
			return AnalyzeStruct(analyzer, dynamic_cast<Struct*>(node));

		default:
			break;
		}

		return TraversalBehavior_AnalyzerSkip;
	}
}