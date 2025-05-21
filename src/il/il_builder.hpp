#ifndef IL_BUILDER_HPP
#define IL_BUILDER_HPP

#include "pch/ast.hpp"
#include "il_instruction.hpp"
#include "il_metadata.hpp"
#include "il_container.hpp"
#include "jump_table.hpp"
#include "il_helper.hpp"
#include "il_expression_builder.hpp"

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
		Statement* statement;
		uint64_t data;
		size_t state;

		ILStatementFrame(Statement* statement, uint64_t data) : statement(statement), data(data), state(0) {}
	};

	struct ILIfFrame
	{
		IfStatement* statement;
		uint64_t nextLocation;
		uint64_t endLocation;
		size_t state;

		ILIfFrame(IfStatement* statement, uint64_t nextLocation) : statement(statement), nextLocation(nextLocation), endLocation(-1), state(0) {}
	};

	struct ILWhileFrame
	{
		WhileStatement* statement;
		uint64_t startLocation;
		uint64_t endLocation;

		ILWhileFrame(WhileStatement* statement, uint64_t startLocation, uint64_t endLocation)
			: statement(statement), startLocation(startLocation), endLocation(endLocation)
		{
		}
	};

	struct ILDoWhileFrame
	{
		DoWhileStatement* statement;
		uint64_t startLocation;
		uint64_t condLocation;
		uint64_t endLocation;

		ILDoWhileFrame(DoWhileStatement* statement, uint64_t startLocation, uint64_t condLocation, uint64_t endLocation)
			: statement(statement), startLocation(startLocation), condLocation(condLocation), endLocation(endLocation)
		{
		}
	};

	struct ILForFrame
	{
		ForStatement* statement;
		uint64_t startLocation;
		uint64_t incrLocation;
		uint64_t endLocation;

		ILForFrame(ForStatement* statement, uint64_t startLocation, uint64_t incrLocation, uint64_t endLocation)
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
			uint64_t location;
		};

		ILFrame(BlockStatement* block, size_t index = 0) : block(ILBlockFrame(block, index)), type(ILFrameType_Block)
		{
		}

		ILFrame(ILFrameType type, Statement* statement, uint64_t state = 0) : statement(ILStatementFrame(statement, state)), type(type)
		{
		}

		ILFrame(IfStatement* statement, uint64_t nextLocation = 0) : ifFrame(ILIfFrame(statement, nextLocation)), type(ILFrameType_IfStatement)
		{
		}

		ILFrame(WhileStatement* statement, uint64_t startLocation, uint64_t endLocation) : whileFrame(ILWhileFrame(statement, startLocation, endLocation)), type(ILFrameType_WhileStatement)
		{
		}

		ILFrame(DoWhileStatement* statement, uint64_t startLocation, uint64_t condLocation, uint64_t endLocation) : doWhileFrame(statement, startLocation, condLocation, endLocation), type(ILFrameType_DoWhileStatement)
		{
		}

		ILFrame(ForStatement* statement, uint64_t startLocation, uint64_t incrLocation, uint64_t endLocation) : forFrame(statement, startLocation, incrLocation, endLocation), type(ILFrameType_ForStatement)
		{
		}

		ILFrame(uint64_t location) : location(location), type(ILFrameType_SetLocation) {}

		ILFrame(ILFrameType type) : type(type) {}

		ILFrame() : type(ILFrameType_Unknown) {}
	};

	struct ILLoopFrame
	{
		uint64_t continueLocation = INVALID_JUMP_LOCATION;
		uint64_t breakLocation = INVALID_JUMP_LOCATION;

		ILLoopFrame() = default;

		ILLoopFrame(uint64_t continueLocation, uint64_t breakLocation)
			: continueLocation(continueLocation), breakLocation(breakLocation)
		{
		}
	};

	class ILBuilder : public ILContainerAdapter, public ILMetadataAdapter
	{
		std::stack<ILFrame> stack;
		ILFrame currentFrame;
		std::stack<ILLoopFrame> loopStack;
		ILLoopFrame currentLoop;

		ILMetadata metadata;
		ILTempVariableAllocator tempAllocator;

		ILExpressionBuilder exprBuilder;

		JumpTable jumpTable;

		ILContainer container;

		std::vector<ILMapping> mapping;
		std::stack<size_t> mappingStarts;

		void MappingStart()
		{
			mappingStarts.push(container.size());
		}

		void MappingEnd(const TextSpan& span, size_t start = -1, size_t end = -1)
		{
			if (start == -1)
			{
				start = mappingStarts.top();
				mappingStarts.pop();
			}
			if (end == -1)
			{
				end = container.size();
			}
			mapping.push_back(ILMapping(static_cast<uint32_t>(start), static_cast<uint32_t>(end - start), span));
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

		ILRegister TraverseExpression(Expression* expr, const ILOperand& outRegister = INVALID_REGISTER) { return exprBuilder.TraverseExpression(expr, outRegister); }
		bool TraverseStatement(Statement* statement);
		void TraverseBlock(ILBlockFrame& frame);
	public:
		ILBuilder() : metadata(tempAllocator), ILContainerAdapter(container), ILMetadataAdapter(metadata), exprBuilder(container, metadata, tempAllocator, jumpTable)
		{
		}

		void Build(FunctionOverload* func);

		const ILMapping* FindMappingForInstruction(size_t instrIndex) const
		{
			int low = 0;
			int high = (int)mapping.size() - 1;

			while (low <= high)
			{
				int mid = low + (high - low) / 2;
				const ILMapping& m = mapping[mid];

				if (instrIndex >= m.start && instrIndex < static_cast<size_t>(m.start) + m.len)
				{
					return &m;
				}
				else if (instrIndex < m.start)
				{
					high = mid - 1;
				}
				else
				{
					low = mid + 1;
				}
			}

			return nullptr;
		}

		void Print()
		{
			std::cout << "{" << std::endl;
			for (size_t i = 0; i < container.size(); i++)
			{
				auto& instr = container[i];
				size_t offset = 0;
				while (true)
				{
					auto it = std::find(jumpTable.locations.begin() + offset, jumpTable.locations.end(), i);
					if (it == jumpTable.locations.end())
					{
						break;
					}
					auto index = std::distance(jumpTable.locations.begin(), it);
					std::cout << "loc_" << index << ":" << std::endl;
					offset = index + 1;
				}
				std::cout << "    " << ToString(instr, metadata) << std::endl;
			}
			std::cout << "}" << std::endl;
		}

		ILContainer& GetContainer()
		{
			return container;
		}

		ILMetadata& GetMetadata()
		{
			return metadata;
		}

		JumpTable& GetJumpTable()
		{
			return jumpTable;
		}

		ILTempVariableAllocator& GetTempAllocator()
		{
			return tempAllocator;
		}
	};
}

#endif