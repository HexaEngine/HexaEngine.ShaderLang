#ifndef OPERAND_FACTORY_HPP
#define OPERAND_FACTORY_HPP

#include "il_instruction.hpp"

namespace HXSL
{
	struct OperandFactory
	{
		BumpAllocator& alloc;

		Operand* operator()(ILVarId id) const
		{
			return alloc.Alloc<Variable>(id);
		}

		Operand* operator()(Number num) const
		{
			return alloc.Alloc<Constant>(num);
		}

		Operand* operator()(ILTypeId typeId) const
		{
			return alloc.Alloc<TypeValue>(typeId);
		}

		Operand* operator()(ILFuncId funcId) const
		{
			return alloc.Alloc<Function>(funcId);
		}

		Operand* operator()(ILLabel label) const
		{
			return alloc.Alloc<Label>(label);
		}

		Operand* operator()(ILFieldAccess field) const
		{
			return alloc.Alloc<FieldAccess>(field);
		}

		Operand* operator()(ILPhiId phiId) const
		{
			return alloc.Alloc<Phi>(phiId);
		}

		ILOpKind operator()(ILOpKind kind) const noexcept { return kind; }

		Operand* operator()(Operand* op) const noexcept
		{
			return op;
		}
	};
}

#endif