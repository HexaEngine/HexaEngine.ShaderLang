#include "optimizers/reassociation_pass.hpp"

namespace HXSL
{
	namespace Backend
	{
        static ResultInstr* FindDef(const dense_map<ILVarId, ResultInstr*>& defs, Operand* op)
        {
            auto* var = dyn_cast<Variable>(op);
            if (!var) return nullptr;
            auto it = defs.find(var->varId);
            return it != defs.end() ? it->second : nullptr;
        }

        struct WorkItem
        {
            Operand* op;
            Number coeffMultiplier;
        };

        static bool ExtractCoefficient(const dense_map<ILVarId, ResultInstr*>& defs, Operand* op, ILVarId targetVarId, Number& outCoefficient)
        {
            std::vector<WorkItem> worklist;
            worklist.push_back({ op, Number(1) });

            Number totalCoeff(0);
            bool foundAny = false;

            while (!worklist.empty())
            {
                auto item = worklist.back();
                worklist.pop_back();

            if (auto* var = dyn_cast<Variable>(item.op))
            {
                if (var->varId == targetVarId)
                {
                    totalCoeff = FoldImm(totalCoeff, item.coeffMultiplier, OpCode_Add);
                    foundAny = true;
                    continue;
                }
            }

            auto* def = FindDef(defs, item.op);
            if (!def) continue;

                if (def->GetOpCode() == OpCode_Multiply)
                {
                    auto* mul = cast<BinaryInstr>(def);
                    auto* lhsVar = dyn_cast<Variable>(mul->GetLHS());
                    auto* rhsConst = dyn_cast<Constant>(mul->GetRHS());

                    if (lhsVar && rhsConst && lhsVar->varId == targetVarId)
                    {
                        Number termCoeff = FoldImm(item.coeffMultiplier, rhsConst->imm(), OpCode_Multiply);
                        totalCoeff = FoldImm(totalCoeff, termCoeff, OpCode_Add);
                        foundAny = true;
                    }
                    continue;
                }

                if (def->GetOpCode() == OpCode_Add)
                {
                    auto* add = cast<BinaryInstr>(def);
                    worklist.push_back({ add->GetLHS(), item.coeffMultiplier });
                    worklist.push_back({ add->GetRHS(), item.coeffMultiplier });
                    continue;
                }

                if (def->GetOpCode() == OpCode_Subtract)
                {
                    auto* sub = cast<BinaryInstr>(def);
                    Number negatedCoeff = FoldImm(Number(0), item.coeffMultiplier, OpCode_Subtract);
                    worklist.push_back({ sub->GetLHS(), item.coeffMultiplier });
                    worklist.push_back({ sub->GetRHS(), negatedCoeff });
                    continue;
                }
            }

            if (foundAny)
            {
                outCoefficient = totalCoeff;
                return true;
            }

            return false;
        }

        static bool TryExtractBaseVar(const dense_map<ILVarId, ResultInstr*>& defs, Operand* op, ILVarId& baseVarId, Variable*& baseVar)
        {
            std::vector<Operand*> worklist;
            worklist.push_back(op);

            while (!worklist.empty())
            {
                auto* current = worklist.back();
                worklist.pop_back();

            auto* def = FindDef(defs, current);
            
            if (!def)
            {
                if (auto* var = dyn_cast<Variable>(current))
                {
                    if (baseVarId == ILVarId(-1))
                    {
                        baseVarId = var->varId;
                        baseVar = var;
                        return true;
                    }
                    if (baseVarId == var->varId)
                    {
                        return true;
                    }
                }
                continue;
            }

                if (def->GetOpCode() == OpCode_Multiply)
                {
                    auto* mul = cast<BinaryInstr>(def);
                    if (auto* var = dyn_cast<Variable>(mul->GetLHS()))
                    {
                        if (dyn_cast<Constant>(mul->GetRHS()))
                        {
                            if (baseVarId == ILVarId(-1))
                            {
                                baseVarId = var->varId;
                                baseVar = var;
                                return true;
                            }
                            if (baseVarId == var->varId)
                            {
                                return true;
                            }
                        }
                    }
                    continue;
                }

                if (def->GetOpCode() == OpCode_Add || def->GetOpCode() == OpCode_Subtract)
                {
                    auto* binOp = cast<BinaryInstr>(def);
                    worklist.push_back(binOp->GetLHS());
                    worklist.push_back(binOp->GetRHS());
                    continue;
                }
            }

            return false;
        }

        static bool TryReassociateMulAddSub(ILContext* context, BinaryInstr& instr, const dense_map<ILVarId, ResultInstr*>& defs)
        {
            /*
                Expression Reassociation Optimization

                Pattern: (x * c1) + (x * c2) => x * (c1 + c2)

                Example:
                    %tmp1_7: float = mul %var1_0: float, 80
                    %tmp1_5: float = add %tmp1_7: float, %tmp1_7: float
                    %tmp1_6: float = mul %var1_0: float, 8000
                    %tmp1_4: float = add %tmp1_5: float, %tmp1_6: float

                Transforms to:
                    %tmp1_4: float = mul %var1_0: float, 8160

                Also handles chained additions:
                    %tmp2_4: float = mul %var1_0: float, 2
                    %var3_1: float = add %var1_0: float, %tmp2_4: float
                    %tmp3_4: float = mul %var1_0: float, 3
                    %var4_1: float = add %var3_1: float, %tmp3_4: float
                    ...

                Transforms to:
                    %var10_1: float = mul %var1_0: float, 45
            */

            bool isSubtract = instr.GetOpCode() == OpCode_Subtract;

            ILVarId baseVarId = ILVarId(-1);
            Variable* baseVar = nullptr;

            bool lhsHasBase = TryExtractBaseVar(defs, instr.GetLHS(), baseVarId, baseVar);
            bool rhsHasBase = TryExtractBaseVar(defs, instr.GetRHS(), baseVarId, baseVar);

            if (!lhsHasBase && !rhsHasBase)
            {
                return false;
            }

            if (baseVarId == ILVarId(-1) || !baseVar)
            {
                return false;
            }

            Number lhsCoeff, rhsCoeff;
            if (!ExtractCoefficient(defs, instr.GetLHS(), baseVarId, lhsCoeff))
            {
                return false;
            }

            if (!ExtractCoefficient(defs, instr.GetRHS(), baseVarId, rhsCoeff))
            {
                return false;
            }

            Number combinedConst = FoldImm(lhsCoeff, rhsCoeff, isSubtract ? OpCode_Subtract : OpCode_Add);
            auto* newConst = context->MakeConstant(Cast(instr, instr.GetResult(), combinedConst));

            instr.OverwriteOpCode(OpCode_Multiply);
            instr.GetLHS() = baseVar;
            instr.GetRHS() = newConst;
            return true;
        }

		void ReassociationPass::Visit(size_t index, BasicBlock& node, EmptyCFGContext& ctx)
		{
			for (auto& instr : node)
			{
                if (auto* resInstr = dyn_cast<ResultInstr>(&instr))
                {
                    definitions[resInstr->GetResult()] = resInstr;
                }

                if (auto* binInstr = dyn_cast<BinaryInstr>(&instr))
                {
                    if (binInstr->GetOpCode() == OpCode_Add || binInstr->GetOpCode() == OpCode_Subtract)
                    {
                        changed |= TryReassociateMulAddSub(context, *binInstr, definitions);
                    }
				}
			}
		}
	}
}