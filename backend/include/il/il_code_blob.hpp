#ifndef IL_CODE_BLOB_HPP
#define IL_CODE_BLOB_HPP

#include "il_metadata.hpp"
#include "jump_table.hpp"
#include "il_container.hpp"
#include <utils/co_trampoline.hpp>

namespace HXSL
{
	class Stream;
	namespace Backend
	{
		class ILCodeBlob
		{
			BumpAllocator allocator;
			ILMetadata metadata;
			JumpTable jumpTable;
			ILContainer instructions;

		public:
			template<typename T>
			using CoTask = TrampolineTask<T>;

			ILCodeBlob() : metadata(allocator), instructions(allocator)
			{
			}

			void FromContext(ILContext* context);
			void Print();
			void Write(Stream* stream, ModuleWriterContext& context);
			CoTask<void> Read(Stream* stream, ModuleReader& context);

			ILContainer& GetInstructions() { return instructions; }
			const ILContainer& GetInstructions() const { return instructions; }
			ILMetadata& GetMetadata() { return metadata; }
			const ILMetadata& GetMetadata() const { return metadata; }
			JumpTable& GetJumpTable() { return jumpTable; }
			const JumpTable& GetJumpTable() const { return jumpTable; }
		};
	}
}

#endif