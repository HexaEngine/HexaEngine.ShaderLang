#ifndef IL_BUILDER_HPP
#define IL_BUILDER_HPP

#include "pch/ast.hpp"
#include "pch/il.hpp"
#include "il_expression_builder.hpp"
#include "middleware/module_builder.hpp"

namespace HXSL
{
	struct ILBlockFrame
	{
		BlockStatement* block;
		size_t index;

		ILBlockFrame(BlockStatement* block, size_t index) : block(block), index(index) {}
	};

	struct ILStatementFrame
	{
		ASTNode* statement;
		uint64_t data;
		size_t state;

		ILStatementFrame(ASTNode* statement, uint64_t data) : statement(statement), data(data), state(0) {}
	};

	struct ILIfFrame
	{
		IfStatement* statement;
		ILLabel nextLocation;
		ILLabel endLocation;
		size_t state;

		ILIfFrame(IfStatement* statement, ILLabel nextLocation) : statement(statement), nextLocation(nextLocation), endLocation(ILLabel(-1)), state(0) {}
	};

	struct ILWhileFrame
	{
		WhileStatement* statement;
		ILLabel startLocation;
		ILLabel endLocation;

		ILWhileFrame(WhileStatement* statement, ILLabel startLocation, ILLabel endLocation)
			: statement(statement), startLocation(startLocation), endLocation(endLocation)
		{
		}
	};

	struct ILDoWhileFrame
	{
		DoWhileStatement* statement;
		ILLabel startLocation;
		ILLabel condLocation;
		ILLabel endLocation;

		ILDoWhileFrame(DoWhileStatement* statement, ILLabel startLocation, ILLabel condLocation, ILLabel endLocation)
			: statement(statement), startLocation(startLocation), condLocation(condLocation), endLocation(endLocation)
		{
		}
	};

	struct ILForFrame
	{
		ForStatement* statement;
		ILLabel startLocation;
		ILLabel incrLocation;
		ILLabel endLocation;

		ILForFrame(ForStatement* statement, ILLabel startLocation, ILLabel incrLocation, ILLabel endLocation)
			: statement(statement), startLocation(startLocation), incrLocation(incrLocation), endLocation(endLocation)
		{
		}
	};

	enum ILFrameType : char
	{
		ILFrameType_Unknown,
		ILFrameType_Block,
		ILFrameType_SingleStatement,
		ILFrameType_IfStatement,
		ILFrameType_WhileStatement,
		ILFrameType_DoWhileStatement,
		ILFrameType_ForStatement,
		ILFrameType_SetLocation,
	};

	struct ILFrame
	{
		ILFrameType type;
		union
		{
			ILBlockFrame block;
			ILStatementFrame statement;
			ILIfFrame ifFrame;
			ILWhileFrame whileFrame;
			ILDoWhileFrame doWhileFrame;
			ILForFrame forFrame;
			ILLabel location;
		};

		ILFrame(BlockStatement* block, size_t index = 0) : block(ILBlockFrame(block, index)), type(ILFrameType_Block)
		{
		}

		ILFrame(ILFrameType type, ASTNode* statement, uint64_t state = 0) : statement(ILStatementFrame(statement, state)), type(type)
		{
		}

		ILFrame(IfStatement* statement, ILLabel nextLocation = ILLabel(-1)) : ifFrame(ILIfFrame(statement, nextLocation)), type(ILFrameType_IfStatement)
		{
		}

		ILFrame(WhileStatement* statement, ILLabel startLocation, ILLabel endLocation) : whileFrame(ILWhileFrame(statement, startLocation, endLocation)), type(ILFrameType_WhileStatement)
		{
		}

		ILFrame(DoWhileStatement* statement, ILLabel startLocation, ILLabel condLocation, ILLabel endLocation) : doWhileFrame(statement, startLocation, condLocation, endLocation), type(ILFrameType_DoWhileStatement)
		{
		}

		ILFrame(ForStatement* statement, ILLabel startLocation, ILLabel incrLocation, ILLabel endLocation) : forFrame(statement, startLocation, incrLocation, endLocation), type(ILFrameType_ForStatement)
		{
		}

		ILFrame(ILLabel location) : location(location), type(ILFrameType_SetLocation) {}

		ILFrame(ILFrameType type) : location(0), type(type) {}

		ILFrame() : location(0), type(ILFrameType_Unknown) {}
	};

	struct ILLoopFrame
	{
		ILLabel continueLocation = INVALID_JUMP_LOCATION;
		ILLabel breakLocation = INVALID_JUMP_LOCATION;

		ILLoopFrame() = default;

		ILLoopFrame(ILLabel continueLocation, ILLabel breakLocation)
			: continueLocation(continueLocation), breakLocation(breakLocation)
		{
		}
	};

	class ILBuilder : public ILContainerAdapter, public ILMetadataAdapter
	{
		ModuleBuilder& builder;
		ILContext* context;
		BumpAllocator* allocator;
		std::stack<ILFrame> stack;
		ILFrame currentFrame;
		std::stack<ILLoopFrame> loopStack;
		ILLoopFrame currentLoop;
		ILExpressionBuilder exprBuilder;
		JumpTable& jumpTable;

		std::stack<Instruction*> mappingStarts;

		void MappingStart()
		{
			mappingStarts.push(&container.back());
		}

		void MappingEnd(const TextSpan& span, Instruction* start = nullptr, Instruction* end = nullptr)
		{
			if (start == nullptr)
			{
				start = mappingStarts.top();
				mappingStarts.pop();
			}
			if (end == nullptr)
			{
				end = &container.back();
			}

			auto location = allocator->Alloc<TextSpan>(span);
			auto current = start->GetNext();
			while (current)
			{
				current->SetLocation(location);
				if (current == end) break;
				current = current->GetNext();
			}
		}

		bool didReturn = false;
		bool pushedCurrent = false;

		void PushCurrent()
		{
			if (currentFrame.type != ILFrameType_Block)
			{
				return;
			}
			if (pushedCurrent)
			{
				return;
			}
			pushedCurrent = true;

			currentFrame.block.index++;
			stack.push(currentFrame);
		}

		void PushFrame(const ILFrame& frame)
		{
			PushCurrent();
			stack.push(frame);
		}

		void PopFrame()
		{
			pushedCurrent = false;
			currentFrame = stack.top();
			stack.pop();
		}

		void PushLoop(const ILLoopFrame& frame)
		{
			loopStack.push(currentLoop);
			currentLoop = frame;
		}

		void PopLoop()
		{
			currentLoop = loopStack.top();
			loopStack.pop();
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

		ILVarId TraverseExpression(Expression* expr, ILVarId outRegister = INVALID_VARIABLE) { return exprBuilder.TraverseExpression(expr, outRegister); }
		bool TraverseStatement(ASTNode* statement);
		void TraverseBlock(ILBlockFrame& frame);
	public:
		ILBuilder(ModuleBuilder& builder, ILContext* context, ILMetadataBuilder& metaBuilder, ILContainer& container, JumpTable& jumpTable)
			: ILContainerAdapter(container), ILMetadataAdapter(metaBuilder),
			builder(builder),
			context(context),
			allocator(&context->GetAllocator()),
			exprBuilder(context, metaBuilder, container, jumpTable),
			jumpTable(jumpTable)
		{
		}

		void Build(FunctionOverload* func);

		void Print()
		{
			std::cout << "{" << std::endl;
			for (auto& instr : container)
			{
				size_t offset = 0;
				while (true)
				{
					auto it = std::find(jumpTable.locations.begin() + offset, jumpTable.locations.end(), &instr);
					if (it == jumpTable.locations.end())
					{
						break;
					}
					auto index = std::distance(jumpTable.locations.begin(), it);
					std::cout << "loc_" << index << ":" << std::endl;
					offset = index + 1;
				}
				std::cout << "    " << ToString(instr, context->GetMetadata()) << std::endl;
			}
			std::cout << "}" << std::endl;
		}
	};
}

#endif