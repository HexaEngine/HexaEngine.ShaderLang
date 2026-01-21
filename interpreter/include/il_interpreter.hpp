#ifndef IL_INTERPRETER_HPP
#define IL_INTERPRETER_HPP

#include "pch/il.hpp"

namespace HXSL
{
    namespace Backend
    {
        class ILInterpreter
        {
        private:

            struct StackFrame
            {
                std::vector<Number> parameters;
                std::vector<Number> registers;
                std::vector<Number> tempRegisters;
                std::stack<Number> stack;
                const ILCodeBlob* codeBlob;
                
                StackFrame(const ILCodeBlob* blob) : codeBlob(blob) {}
            };

            std::vector<StackFrame> callStack;
            bool shouldReturn = false;
            Number returnValue;

        public:
            ILInterpreter() = default;

            Number Execute(const ILCodeBlob* blob, const std::vector<Number>& params = {})
            {
                shouldReturn = false;
                returnValue = Number();
                callStack.clear();

                PushFrame(blob);
                
                auto& frame = CurrentFrame();
                frame.parameters = params;

                const auto& instructions = blob->GetInstructions();

                for (auto& instr : instructions)
                {
                    if (shouldReturn)
                        break;

                    ExecuteInstruction(&instr);
                }

                return returnValue;
            }

        private:
            StackFrame& CurrentFrame()
            {
                return callStack.back();
            }

            const StackFrame& CurrentFrame() const
            {
                return callStack.back();
            }

            void PushFrame(const ILCodeBlob* blob)
            {
                callStack.emplace_back(blob);
                auto& frame = callStack.back();

                const auto& metadata = blob->GetMetadata();
                const auto& variables = metadata.variables;
                const auto& tempVariables = metadata.tempVariables;

                frame.registers.resize(variables.size());
                frame.tempRegisters.resize(tempVariables.size());

                for (size_t i = 0; i < frame.registers.size(); ++i)
                {
                    frame.registers[i] = Number();
                }

                for (size_t i = 0; i < frame.tempRegisters.size(); ++i)
                {
                    frame.tempRegisters[i] = Number();
                }
            }

            void PopFrame()
            {
                callStack.pop_back();
            }

            Number GetOperandValue(const Operand* operand)
            {
                if (!operand)
                    return Number();

                if (Operand::IsImm(operand))
                {
                    return cast<Constant>(operand)->imm();
                }
                else if (Operand::IsVar(operand))
                {
                    auto& frame = CurrentFrame();
                    auto var = cast<Variable>(operand);
                    uint64_t index = var->varId.var.id;
                    bool isTemp = var->varId.var.temp;

                    if (isTemp)
                    {
                        return frame.tempRegisters[index];
                    }
                    else
                    {
                        return frame.registers[index];
                    }
                }

                return Number();
            }

            void SetRegister(const ILVarId& varId, const Number& value)
            {
                auto& frame = CurrentFrame();
                uint64_t index = varId.var.id;
                bool isTemp = varId.var.temp;

                if (isTemp)
                {
                    frame.tempRegisters[index] = value;
                }
                else
                {
                    frame.registers[index] = value;
                }
            }

            void ExecuteInstruction(const Instruction* instr)
            {
                ILOpCode opcode = instr->GetOpCode();

                switch (opcode)
                {
                case OpCode_Noop:
                    break;

                case OpCode_Move:
                {
                    auto moveInstr = cast<MoveInstr>(instr);
                    Number src = GetOperandValue(moveInstr->GetSource());
                    SetRegister(moveInstr->GetResult(), src);
                    break;
                }

                case OpCode_Return:
                {
                    auto retInstr = cast<ReturnInstr>(instr);
                    Operand* retVal = retInstr->GetReturnValue();
                    if (retVal)
                    {
                        returnValue = GetOperandValue(retVal);
                    }
                    shouldReturn = true;
                    break;
                }

                case OpCode_Add:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs + rhs);
                    break;
                }

                case OpCode_Subtract:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs - rhs);
                    break;
                }

                case OpCode_Multiply:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs * rhs);
                    break;
                }

                case OpCode_Divide:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs / rhs);
                    break;
                }

                case OpCode_Modulus:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs % rhs);
                    break;
                }

                case OpCode_BitwiseAnd:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs & rhs);
                    break;
                }

                case OpCode_BitwiseOr:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs | rhs);
                    break;
                }

                case OpCode_BitwiseXor:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs ^ rhs);
                    break;
                }

                case OpCode_BitwiseShiftLeft:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs << rhs);
                    break;
                }

                case OpCode_BitwiseShiftRight:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), lhs >> rhs);
                    break;
                }

                case OpCode_LessThan:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)(lhs < rhs)));
                    break;
                }

                case OpCode_LessThanOrEqual:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)(lhs <= rhs)));
                    break;
                }

                case OpCode_GreaterThan:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)(lhs > rhs)));
                    break;
                }

                case OpCode_GreaterThanOrEqual:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)(lhs >= rhs)));
                    break;
                }

                case OpCode_Equal:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)(lhs == rhs)));
                    break;
                }

                case OpCode_NotEqual:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)(lhs != rhs)));
                    break;
                }

                case OpCode_AndAnd:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)((lhs.u8 != 0) && (rhs.u8 != 0))));
                    break;
                }

                case OpCode_OrOr:
                {
                    auto binInstr = cast<BinaryInstr>(instr);
                    Number lhs = GetOperandValue(binInstr->GetLHS());
                    Number rhs = GetOperandValue(binInstr->GetRHS());
                    SetRegister(binInstr->GetResult(), Number((uint8_t)((lhs.u8 != 0) || (rhs.u8 != 0))));
                    break;
                }

                case OpCode_Negate:
                {
                    auto unaryInstr = cast<UnaryInstr>(instr);
                    Number op = GetOperandValue(unaryInstr->GetOperand());
                    SetRegister(unaryInstr->GetResult(), -op);
                    break;
                }

                case OpCode_LogicalNot:
                {
                    auto unaryInstr = cast<UnaryInstr>(instr);
                    Number op = GetOperandValue(unaryInstr->GetOperand());
                    SetRegister(unaryInstr->GetResult(), Number((uint8_t)(op.u8 == 0)));
                    break;
                }

                case OpCode_BitwiseNot:
                {
                    auto unaryInstr = cast<UnaryInstr>(instr);
                    Number op = GetOperandValue(unaryInstr->GetOperand());
                    SetRegister(unaryInstr->GetResult(), ~op);
                    break;
                }

                case OpCode_Increment:
                {
                    auto unaryInstr = cast<UnaryInstr>(instr);
                    Number op = GetOperandValue(unaryInstr->GetOperand());
                    SetRegister(unaryInstr->GetResult(), op + Number((uint32_t)1));
                    break;
                }

                case OpCode_Decrement:
                {
                    auto unaryInstr = cast<UnaryInstr>(instr);
                    Number op = GetOperandValue(unaryInstr->GetOperand());
                    SetRegister(unaryInstr->GetResult(), op - Number((uint32_t)1));
                    break;
                }

                case OpCode_Push:
                {
                    auto& frame = CurrentFrame();
                    auto operand = instr->GetOperand(0);
                    Number value = GetOperandValue(operand);
                    frame.stack.push(value);
                    break;
                }

                case OpCode_Pop:
                {
                    auto& frame = CurrentFrame();
                    auto resultInstr = cast<ResultInstr>(instr);
                    Number value = frame.stack.top();
                    frame.stack.pop();
                    SetRegister(resultInstr->GetResult(), value);
                    break;
                }

                case OpCode_Load:
                {
                    auto loadInstr = cast<LoadInstr>(instr);
                    Number value = GetOperandValue(loadInstr->GetOperand(0));
                    SetRegister(loadInstr->GetResult(), value);
                    break;
                }

                case OpCode_Store:
                {
                    auto storeInstr = cast<StoreInstr>(instr);
                    Number value = GetOperandValue(storeInstr->GetSource());
                    if (Operand::IsVar(storeInstr->GetDestination()))
                    {
                        auto var = cast<Variable>(storeInstr->GetDestination());
                        SetRegister(var->varId, value);
                    }
                    break;
                }

                case OpCode_Cast:
                {
                    auto unaryInstr = cast<UnaryInstr>(instr);
                    Number op = GetOperandValue(unaryInstr->GetOperand());
                    SetRegister(unaryInstr->GetResult(), op);
                    break;
                }

                case OpCode_Call:
                {
                    auto callInstr = cast<CallInstr>(instr);
                    auto func = callInstr->GetFunction();
                    auto funcLayout = func->funcId->func;
                    auto targetBlob = funcLayout->GetCodeBlob();
                    
                    if (targetBlob)
                    {
                        PushFrame(targetBlob);
                        
                        const auto& instructions = targetBlob->GetInstructions();
                        bool prevReturn = shouldReturn;
                        shouldReturn = false;
                        
                        for (auto& targetInstr : instructions)
                        {
                            if (shouldReturn)
                                break;
                            
                            ExecuteInstruction(&targetInstr);
                        }
                        
                        Number result = returnValue;
                        shouldReturn = prevReturn;
                        
                        PopFrame();
                        
                        if (callInstr->GetResult().raw != INVALID_VARIABLE.raw)
                        {
                            SetRegister(callInstr->GetResult(), result);
                        }
                    }
                    break;
                }

                case OpCode_LoadParam:
                {
                    auto loadParamInstr = cast<LoadParamInstr>(instr);
                    auto& frame = CurrentFrame();
                    size_t paramIdx = loadParamInstr->GetParamIdx();
                    
                    SetRegister(loadParamInstr->GetResult(), frame.parameters[paramIdx]);
                    break;
                }

                case OpCode_StoreParam:
                {
                    auto storeParamInstr = cast<StoreParamInstr>(instr);
                    size_t paramIdx = storeParamInstr->GetParamIdx();
                    Number value = GetOperandValue(storeParamInstr->GetSource());
                    
                    if (callStack.size() > 1)
                    {
                        auto& nextFrame = callStack[callStack.size() - 1];
                        if (paramIdx >= nextFrame.parameters.size())
                        {
                            nextFrame.parameters.resize(paramIdx + 1);
                        }
                        nextFrame.parameters[paramIdx] = value;
                    }
                    break;
                }

                case OpCode_Discard:
                    shouldReturn = true;
                    break;

                default:
                    break;
                }
            }
        };
    }
}

#endif
