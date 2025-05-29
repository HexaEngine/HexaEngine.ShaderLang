#ifndef OPERAND_FACTORY_HPP
#define OPERAND_FACTORY_HPP

#include "il_instruction.hpp"

namespace HXSL
{
	struct OperandFactory
	{
		BumpAllocator& alloc;

		Variable* operator()(ILVarId id) const
		{
			return alloc.Alloc<Variable>(id);
		}

		Constant* operator()(Number num) const
		{
			return alloc.Alloc<Constant>(num);
		}

		TypeValue* operator()(ILType typeId) const
		{
			return alloc.Alloc<TypeValue>(typeId);
		}

		Function* operator()(ILFuncId funcId) const
		{
			return alloc.Alloc<Function>(funcId);
		}

		Label* operator()(ILLabel label) const
		{
			return alloc.Alloc<Label>(label);
		}

		FieldAccess* operator()(ILFieldAccess field) const
		{
			return alloc.Alloc<FieldAccess>(field);
		}

		Variable* operator()(Variable* op) const noexcept
		{
			return alloc.Alloc<Variable>(op->varId);
		}

		Operand* operator()(Operand* op) const noexcept
		{
			return op;
		}
	};
}

#endif