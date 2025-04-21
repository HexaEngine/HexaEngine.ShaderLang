#include "declaration_analyzer.hpp"

namespace HXSL
{
	static HXSLTraversalBehavior AnalyzeStruct(HXSLAnalyzer& analyzer, HXSLStruct* node)
	{
		std::stack<HXSLStruct*> traversalStack;
		std::unordered_set<HXSLStruct*> visited;
		std::unordered_set<HXSLStruct*> processing;

		traversalStack.push(node);

		while (!traversalStack.empty())
		{
			HXSLStruct* currentNode = traversalStack.top();

			if (processing.find(currentNode) != processing.end())
			{
				analyzer.LogError("Recursive struct layout detected.", currentNode->GetSpan());
				return HXSLTraversalBehavior_Break;
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

				if (auto fieldStruct = dynamic_cast<HXSLStruct*>(decl))
				{
					if (visited.find(fieldStruct) == visited.end())
					{
						traversalStack.push(fieldStruct);
						allFieldsProcessed = false;
					}
					else
					{
						analyzer.LogError("Recursive struct layout detected.", currentNode->GetSpan());
						return HXSLTraversalBehavior_Break;
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

		return HXSLTraversalBehavior_AnalyzerSkip;
	}

	HXSLTraversalBehavior HXSLDeclarationAnalyzer::Analyze(HXSLAnalyzer& analyzer, HXSLNode* node, HXSLCompilation* compilation)
	{
		auto& type = node->GetType();

		switch (type)
		{
		case HXSLNodeType_Struct:
			return AnalyzeStruct(analyzer, dynamic_cast<HXSLStruct*>(node));

		default:
			break;
		}

		return HXSLTraversalBehavior_AnalyzerSkip;
	}
}