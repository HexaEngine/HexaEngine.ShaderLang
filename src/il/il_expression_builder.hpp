#ifndef IL_EXPRESSION_BUILDER_HPP
#define IL_EXPRESSION_BUILDER_HPP

#include "ast_ilgen.hpp"
#include "pch/il.hpp"

namespace HXSL
{
	struct ILExpressionFrame
	{
		Expression* expression = nullptr;
		Operand* outRegister = nullptr;
		Operand* rightRegister = nullptr;
		Operand* leftRegister = nullptr;
		uint64_t state = 0;
		ILLabel label = INVALID_JUMP_LOCATION;

		ILExpressionFrame(Expression* expression, Operand* reg, Operand* rightRegister = nullptr, Operand* leftRegister = nullptr)
			: expression(expression), outRegister(reg), rightRegister(rightRegister), leftRegister(leftRegister), state(0), label(INVALID_JUMP_LOCATION)
		{
		}

		ILExpressionFrame() = default;
	};

	enum MemberOp
	{
		MemberOp_None,
		MemberOp_Read,
		MemberOp_Write,
	};

	class ILExpressionBuilder : public ILContainerAdapter, public ILMetadataAdapter
	{
		ILContext* context;
		LowerCompilationUnit* compilation;
		std::stack<ILExpressionFrame> stack;
		ILExpressionFrame currentFrame;
		ILTempVariableAllocator& reg;
		JumpTable& jumpTable;

		void PushCurrent()
		{
			stack.push(currentFrame);
		}

		void PushFrame(const ILExpressionFrame& frame)
		{
			stack.push(frame);
		}

		void PopFrame()
		{
			currentFrame = stack.top();
			stack.pop();
		}

		void SetLocation(ILLabel label, ILInstruction* location = INVALID_JUMP_LOCATION_PTR)
		{
			if (location == INVALID_JUMP_LOCATION_PTR)
			{
				location = &container.back();
			}
			jumpTable.SetLocation(label, location);
		}

		ILLabel MakeJumpLocation(ILInstruction* location = INVALID_JUMP_LOCATION_PTR)
		{
			return jumpTable.Allocate(location);
		}

		ILLabel MakeJumpLocationFromCurrent() { return jumpTable.Allocate(&container.back()); }

	public:
		ILExpressionBuilder(ILContext* context, LowerCompilationUnit* compilation, ILContainer& container, ILMetadata& metadata, ILTempVariableAllocator& tempAllocator, JumpTable& jumpTable)
			: ILContainerAdapter(container), ILMetadataAdapter(context->GetMetadata()),
			context(context),
			compilation(context->compilation),
			jumpTable(jumpTable),
			reg(tempAllocator)
		{
		}

		bool IsInlineable(Expression* expr, Operand*& opOut);
		void ReadVar(Expression* target, Operand* varOut);
		void WriteVar(Expression* target, Operand* varIn);
		SymbolDef* GetAddrType(SymbolDef* elementType);
		bool MemberAccess(Expression* expr, Operand*, MemberOp op = MemberOp_Read, Operand* writeOp = nullptr);
		void FunctionCall(FunctionCallExpression* expr);
		void OperatorCall(OperatorOverload* op, Operand* left, Operand* right, Operand* result);
		void OperatorCall(BinaryExpression* binary, Operand* left, Operand* right, Operand* result)
		{
			auto op = binary->GetOperatorDeclaration();
			if (op == nullptr) return;
			return OperatorCall(op->As<OperatorOverload>(), left, right, result);
		}
		Operand* TraverseExpression(Expression* expression, Operand* outOperand = nullptr);
	};
}

#endif