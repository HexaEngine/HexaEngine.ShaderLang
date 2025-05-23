#ifndef IL_EXPRESSION_BUILDER_HPP
#define IL_EXPRESSION_BUILDER_HPP

#include "pch/ast.hpp"
#include "pch/il.hpp"
#include "il_temp_var_allocator.hpp"

namespace HXSL
{
	struct ILExpressionFrame
	{
		Expression* expression = nullptr;
		ILOperand outRegister = INVALID_REGISTER;
		ILRegister rightRegister = INVALID_REGISTER;
		ILRegister leftRegister = INVALID_REGISTER;
		uint64_t state = 0;
		uint64_t data = 0;

		ILExpressionFrame(Expression* expression, const ILOperand& reg, ILRegister rightRegister = INVALID_REGISTER, ILRegister leftRegister = INVALID_REGISTER)
			: expression(expression), outRegister(reg), rightRegister(rightRegister), leftRegister(leftRegister), state(0), data(0)
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

		void SetLocation(uint64_t label, uint64_t location = INVALID_JUMP_LOCATION)
		{
			if (location == INVALID_JUMP_LOCATION)
			{
				location = container.size();
			}
			jumpTable.SetLocation(label, location);
		}

		uint64_t MakeJumpLocation(uint64_t location = INVALID_JUMP_LOCATION)
		{
			return jumpTable.Allocate(location);
		}

		uint64_t MakeJumpLocationFromCurrent() { return jumpTable.Allocate(container.size()); }

	public:
		ILExpressionBuilder(ILContainer& container, ILMetadata& metadata, ILTempVariableAllocator& tempAllocator, JumpTable& jumpTable) : ILContainerAdapter(container), ILMetadataAdapter(metadata), jumpTable(jumpTable), reg(tempAllocator)
		{
		}

		bool IsInlineable(Expression* expr, ILOperand& opOut);
		void ReadVar(Expression* target, ILRegister registerOut);
		void WriteVar(Expression* target, ILRegister registerIn);
		bool MemberAccess(Expression* expr, ILRegister outRegister, MemberOp op = MemberOp_Read, ILOperand writeOp = {});
		void FunctionCall(FunctionCallExpression* expr);
		void OperatorCall(OperatorOverload* op, const ILOperand& left, const ILOperand& right, const ILOperand& result);
		void OperatorCall(BinaryExpression* binary, const ILOperand& left, const ILOperand& right, const ILOperand& result)
		{
			auto op = binary->GetOperatorDeclaration();
			if (op == nullptr) return;
			return OperatorCall(op->As<OperatorOverload>(), left, right, result);
		}
		ILRegister TraverseExpression(Expression* expression, const ILOperand& outOperand = INVALID_REGISTER);
	};
}

#endif