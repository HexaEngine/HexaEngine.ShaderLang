#include "optimizers/loop_unroller.hpp"
#include "il/il_helper.hpp"

namespace HXSL
{
    namespace Backend
    {
        void LoopUnroller::MapVariable(Operand*& operand)
        {
            if (auto var = dyn_cast<Variable>(operand))
            {
                auto it = varMap.find(var->varId.StripVersion());
                if (it != varMap.end())
                {
                    operand = context->MakeVariable(it->second);
                }
            }
        }

        void LoopUnroller::MapVariable(LoopAnalysis& analysis, Operand*& operand, size_t iteration)
        {
            if (auto var = dyn_cast<Variable>(operand))
            {
                if (var->varId == analysis.inductionVar)
                {
                    // Calculate: start + (step * i)
                    Number iterationIndex = Number((int32_t)iteration);
                    Number offset = analysis.stepValue * iterationIndex;
                    Number iterValue = analysis.startValue + offset;
                    operand = context->MakeConstant(iterValue);
                }
                else
                {
					return MapVariable(operand);
                }
            }
        }

        ILVarId LoopUnroller::VersionVariable(ILVarId varId)
        {
            auto canonicalId = varId.StripVersion();
            auto it = varMap.find(canonicalId);
            if (it == varMap.end())
            {
                varMap[canonicalId] = varId;
                return varId;
            }
            else
            {
                auto newResult = it->second;
                newResult.var.version++;
                varMap[canonicalId] = newResult;
                return newResult;
            }
        }

        void LoopUnroller::MapInstruction(LoopAnalysis& analysis, Instruction*& instr, size_t iteration)
        {
            for (auto& operand : instr->GetOperands())
            {
                MapVariable(analysis, operand, iteration);
            }

            if (auto resultInstr = dyn_cast<ResultInstr>(instr))
            {
                resultInstr->SetResult(VersionVariable(resultInstr->GetResult()));
            }
        }

        LoopUnroller::LoopAnalysis LoopUnroller::AnalyzeLoop(LoopNode* loop, ControlFlowGraph& cfg)
        {
            LoopAnalysis analysis = {};

            BasicBlock* header = loop->GetHeader();
            BasicBlock* preHeader = loop->GetPreHeader();

            if (!preHeader || header->NumPredecessors() != 2)
            {
                return analysis;
            }

            PhiInstr* inductionPhi = nullptr;
            for (auto& instr : *header)
            {
                if (auto phi = dyn_cast<PhiInstr>(&instr))
                {
                    auto initOperand = phi->GetOperand(0);

                    if (auto initVar = dyn_cast<Variable>(initOperand))
                    {
                        // Find the instruction in preheader that defines this variable
                        Instruction* defInstr = nullptr;
                        for (auto& preHeaderInstr : *preHeader)
                        {
                            if (auto resultInstr = dyn_cast<ResultInstr>(&preHeaderInstr))
                            {
                                if (resultInstr->GetResult() == initVar->varId)
                                {
                                    defInstr = &preHeaderInstr;
                                    break;
                                }
                            }
                        }

                        // Check if it's a mov instruction with a constant value
                        if (auto moveInstr = dyn_cast<MoveInstr>(defInstr))
                        {
                            if (auto constOp = dyn_cast<Constant>(moveInstr->GetSource()))
                            {
                                if (constOp->imm().IsIntegral())
                                {
                                    inductionPhi = phi;
                                    analysis.inductionVarPhi = phi;
                                    analysis.inductionVar = phi->GetResult();
                                    analysis.startValue = constOp->imm();
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if (!inductionPhi)
            {
                return analysis;
            }

            BinaryInstr* compareInstr = nullptr;
            for (auto& instr : *header)
            {
                if (auto binOp = dyn_cast<BinaryInstr>(&instr))
                {
                    auto opcode = binOp->GetOpCode();
                    if (opcode == OpCode_LessThan || opcode == OpCode_LessThanOrEqual ||
                        opcode == OpCode_GreaterThan || opcode == OpCode_GreaterThanOrEqual)
                    {
                        if (auto lhsVar = dyn_cast<Variable>(binOp->GetLHS()))
                        {
                            if (lhsVar->varId == analysis.inductionVar)
                            {
                                if (auto rhsConst = dyn_cast<Constant>(binOp->GetRHS()))
                                {
                                    compareInstr = binOp;
                                    analysis.compareOp = opcode;
                                    analysis.endValue = rhsConst->imm();
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if (!compareInstr)
            {
                return analysis;
            }

            if (loop->GetLatches().size() != 1)
            {
                return analysis;
            }

            BasicBlock* latch = *loop->GetLatches().begin();
            for (auto& instr : *latch)
            {
                if (auto binOp = dyn_cast<BinaryInstr>(&instr))
                {
                    if (binOp->GetOpCode() == OpCode_Add)
                    {
                        if (auto rhsConst = dyn_cast<Constant>(binOp->GetRHS()))
                        {
                            analysis.stepValue = rhsConst->imm();

                            // Calculate trip count: (end - start) / step
                            // Do all arithmetic in Number space, then convert final result
                            Number range = analysis.endValue - analysis.startValue;
                            Number iterCount = range / analysis.stepValue;

                            if (analysis.compareOp == OpCode_LessThan)
                            {
                                analysis.tripCount = iterCount.ToSizeT();
                            }
                            else if (analysis.compareOp == OpCode_LessThanOrEqual)
                            {
                                analysis.tripCount = iterCount.ToSizeT() + 1;
                            }

                            // Only unroll if trip count is reasonable and positive
                            if (analysis.tripCount > 0 && analysis.tripCount <= 16)
                            {
                                analysis.isValidForUnroll = true;
                            }
                            return analysis;
                        }
                    }
                }
            }

            return analysis;
        }

        bool LoopUnroller::UnrollLoop(LoopNode* loop, LoopAnalysis& analysis, ILContext* context)
        {
            auto& cfg = context->GetCFG();
            auto& allocator = context->GetAllocator();

            BasicBlock* header = loop->GetHeader();
            BasicBlock* preHeader = loop->GetPreHeader();
            BasicBlock* latch = *loop->GetLatches().begin();
            BasicBlock* exitBlock = *loop->GetExits().begin();

            BasicBlock* bodyBlock = nullptr;
            for (auto* block : loop->GetBlocks())
            {
                if (block != header && block != latch)
                {
                    bodyBlock = block;
                    break;
                }
            }

            if (!bodyBlock)
            {
                return false;
            }

            varMap.clear();
            
            for (auto& instr : *header)
            {
                if (auto phi = dyn_cast<PhiInstr>(&instr))
                {
                    auto initOperand = phi->GetOperand(0);
                    if (auto initVar = dyn_cast<Variable>(initOperand))
                    {
                        auto canonicalId = phi->GetResult().StripVersion();
                        varMap[canonicalId] = initVar->varId;
                    }
                }
            }

            for (uint64_t i = 0; i < analysis.tripCount; ++i)
            {
                for (auto& instr : *bodyBlock)
                {
                    auto clonedInstr = instr.Clone(allocator);
                    MapInstruction(analysis, clonedInstr, i);
                    preHeader->AddInstr(clonedInstr);
                }


                // Copy latch instructions EXCEPT the induction variable increment
                if (i < analysis.tripCount - 1)
                {
                    for (auto& instr : *latch)
                    {
                        if (isa<JumpInstr>(&instr))
                        {
                            continue;
                        }

                        // Skip the instruction that defines the induction variable
                        if (auto resultInstr = dyn_cast<ResultInstr>(&instr))
                        {
                            if (resultInstr->GetResult().StripVersion() == analysis.inductionVar.StripVersion())
                            {
                                continue;
                            }
                        }

                        auto clonedInstr = instr.Clone(allocator);
                        MapInstruction(analysis, clonedInstr, i);
                        preHeader->AddInstr(clonedInstr);
                    }
                }
            }

            for (auto& instr : *exitBlock)
            {
                for (auto& operand : instr.GetOperands())
                {
                    MapVariable(operand); 
                }
            }

            cfg.Unlink(preHeader->GetId(), header->GetId());
            cfg.Link(preHeader->GetId(), exitBlock->GetId());

			for (auto block : loop->GetBlocks())
            {
                cfg.RemoveNode(block->GetId());
            }

            if (exitBlock->GetPredecessors().size() == 1)
            {
                cfg.MergeNodes(preHeader->GetId(), exitBlock->GetId());
            }

            cfg.RebuildDomTree();

            return true;
        }

        OptimizerPassResult LoopUnroller::Run()
        {
            /*

Node 0 [Normal]
  Instructions:
    %var1_0: float = ldarg 0
    %var1_1: float = mov 0                                                     ;  (Line: 5 Column: 1)
    %var1_2: int = mov 0                                                       ;  (Line: 6 Column: 5)
  Predecessors:
  Successors: 1
Node 1 [Conditional]
  Instructions:
    %var2_2: int = phi %var1_2: int, %var3_2: int
    %var2_1: float = phi %var1_1: float, %var3_1: float
    %tmp1_1: bool = lt %var2_2: int, 10                                        ;  (Line: 6 Column: 12)
    jz #loc_4
  Predecessors: 0 3
  Successors: 4 2
Node 2 [Normal]
  Instructions:
    %tmp1_5: float = cast %var2_2: int                                         ;  (Line: 8 Column: 1)
    %tmp1_4: float = add %var1_0: float, %tmp1_5: float                        ;  (Line: 8 Column: 1)
    %var3_1: float = add %var2_1: float, %tmp1_4: float                        ;  (Line: 8 Column: 1)
  Predecessors: 1
  Successors: 3
Node 3 [Unconditional]
  Instructions:
    %var3_2: int = add %var2_2: int, 1
    jmp #loc_1
  Predecessors: 2
  Successors: 1
Node 4 [Exit]
  Instructions:
    ret %var2_1: float                                                         ;  (Line: 10 Column: 1)
  Predecessors: 1
  Successors:

Loop Tree:
LoopNode(Header: 1, Depth: 0)
  PreHeader: 0
  Blocks: 1 2 3
  Latches: 3
  Exits: 4


            */

            auto& loopTree = context->GetLoopTree();
            auto& cfg = context->GetCFG();
            bool madeChange = false;

            for (auto& loopNode : loopTree.GetNodes())
            {
                if (loopNode->GetDepth() != 0)
                {
                    continue;
                }

                auto analysis = AnalyzeLoop(loopNode.get(), cfg);
                if (analysis.isValidForUnroll)
                {
                    if (UnrollLoop(loopNode.get(), analysis, context))
                    {
                        madeChange = true;
                    }
                }
            }

            if (madeChange)
            {
                return OptimizerPassResult_Changed;
            }
            return OptimizerPassResult_None;
        }
    }
}