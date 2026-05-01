#include "declaration_analyzer.hpp"
#include "utils/expression_evaluator.hpp"

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

				if (auto fieldStruct = dyn_cast<Struct>(decl))
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

	TraversalBehavior DeclarationAnalyzer::Analyze(SemanticAnalyzer& analyzer, ASTNode* node, CompilationUnit* compilation)
	{
		auto type = node->GetType();

		switch (type)
		{
		case NodeType_Struct:
			return AnalyzeStruct(analyzer, cast<Struct>(node));

		default:
			break;
		}

		return TraversalBehavior_AnalyzerSkip;
	}

	static bool IsIntegral(const Primitive* prim)
	{
		if (prim->GetClass() != PrimitiveClass_Scalar)
		{
			return false;
		}
		switch (prim->GetKind())
		{
		case PrimitiveKind_Int8:
		case PrimitiveKind_UInt8:
		case PrimitiveKind_Int16:
		case PrimitiveKind_UInt16:
		case PrimitiveKind_Int:
		case PrimitiveKind_UInt:
		case PrimitiveKind_Int64:
		case PrimitiveKind_UInt64:
			return true;
		default:
			return false;
		}
	}

	static NumberType GetEnumItemType(const Primitive* prim)
	{
		switch (prim->GetKind())
		{
		case PrimitiveKind_Int8:
			return NumberType_Int8;
		case PrimitiveKind_UInt8:
			return NumberType_UInt8;
		case PrimitiveKind_Int16:
			return NumberType_Int16;
		case PrimitiveKind_UInt16:
			return NumberType_UInt16;
		case PrimitiveKind_Int:
			return NumberType_Int32;
		case PrimitiveKind_UInt:
			return NumberType_UInt32;
		case PrimitiveKind_Int64:
			return NumberType_Int64;
		case PrimitiveKind_UInt64:
			return NumberType_UInt64;
		default:
			HXSL_ASSERT(false, "Invalid enum base type.");
			return NumberType_Int32; // Default to int32 for error recovery.
		}
	}

	TraversalBehavior EnumAnalyzer::Analyze(SemanticAnalyzer& analyzer, ASTNode* node, CompilationUnit* compilation)
	{
		auto enumDef = cast<Enum>(node);

		auto baseType = enumDef->GetBaseType();
		auto prim = dyn_cast<Primitive>(baseType);
		if (!prim || !IsIntegral(prim))
		{
			analyzer.Log(INVALID_ENUM_BASE_TYPE, enumDef->GetSpan());
			return TraversalBehavior_Break;
		}
		
		auto numberType = GetEnumItemType(prim);
		Number next = Number({}, numberType);

		ExpressionEvaluatorContext ctx;
		for (auto& item : enumDef->GetItems())
		{
			auto expr = item->GetValue();
			if (expr)
			{
				auto result = ExpressionEvaluator::TryEvaluate(expr, ctx);
				auto value = result.value;
				item->SetComputedValue(value.u64);
				next = value;
			}
			else
			{
				item->SetComputedValue(next.u64);
			}
			ctx.symbols[item] = next;
			next++;
		}

		return TraversalBehavior_AnalyzerSkip;
	}
}