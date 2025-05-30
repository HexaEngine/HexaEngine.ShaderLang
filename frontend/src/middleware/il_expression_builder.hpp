#ifndef IL_EXPRESSION_BUILDER_HPP
#define IL_EXPRESSION_BUILDER_HPP

#include "pch/ast.hpp"
#include "pch/il.hpp"
#include "metadata_builder.hpp"

namespace HXSL
{
	using namespace Backend;
	using ILContext = Backend::ILContext;

	struct ILExpressionFrame
	{
		Expression* expression = nullptr;
		ILVarId outRegister = INVALID_VARIABLE;
		Variable* rightRegister = nullptr;
		Variable* leftRegister = nullptr;
		uint64_t state = 0;
		ILLabel label = INVALID_JUMP_LOCATION;

		ILExpressionFrame(Expression* expression, ILVarId reg, Variable* rightRegister = nullptr, Variable* leftRegister = nullptr)
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
		Backend::Module* module;
		std::stack<ILExpressionFrame> stack;
		ILExpressionFrame currentFrame;
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

		void SetLocation(ILLabel label, Instruction* location = INVALID_JUMP_LOCATION_PTR)
		{
			if (location == INVALID_JUMP_LOCATION_PTR)
			{
				location = &container.back();
			}
			jumpTable.SetLocation(label, location);
		}

		ILLabel MakeJumpLocation(Instruction* location = INVALID_JUMP_LOCATION_PTR)
		{
			return jumpTable.Allocate(location);
		}

		ILLabel MakeJumpLocationFromCurrent() { return jumpTable.Allocate(&container.back()); }

	public:
		ILExpressionBuilder(ILContext* context, ILMetadataBuilder& metaBuilder, ILContainer& container, JumpTable& jumpTable)
			: ILContainerAdapter(container), ILMetadataAdapter(metaBuilder),
			context(context),
			module(context->GetModule()),
			jumpTable(jumpTable)
		{
		}

		bool IsInlineable(Expression* expr, Operand*& opOut);
		void ReadVar(Expression* target, ILVarId varOut);
		void WriteVar(Expression* target, Operand* varIn);
		ILVariable& MemberAccess(Expression* expr, bool& isAddress);
		void FunctionCall(FunctionCallExpression* expr);
		void OperatorCall(OperatorOverload* op, Operand* left, Operand* right, ILVarId result);
		void OperatorCall(BinaryExpression* binary, Operand* left, Operand* right, ILVarId result)
		{
			auto op = binary->GetOperatorDeclaration();
			if (op == nullptr) return;
			return OperatorCall(op->As<OperatorOverload>(), left, right, result);
		}
		ILVarId TraverseExpression(Expression* expression, ILVarId outOperand = INVALID_VARIABLE);
	};
}

#endif