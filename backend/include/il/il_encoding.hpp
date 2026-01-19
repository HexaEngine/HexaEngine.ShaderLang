#ifndef IL_ENCODING_HPP
#define IL_ENCODING_HPP

#include "instruction.hpp"
#include "il_metadata.hpp"
#include "io/stream.hpp"

namespace HXSL
{
    namespace Backend
    {
        enum class OpKind : uint8_t
        {
            Disabled = 0,
            Variable = 1,
            ImmU8 = 2,
            ImmU16 = 3,
            ImmU32 = 4,
            ImmU64 = 5,
            ImmI8 = 6,
            ImmI16 = 7,
            ImmI32 = 8,
            ImmI64 = 9,
            ImmF16 = 10,
            ImmF32 = 11,
            ImmF64 = 12,
            Label = 13,
            Type = 14,
            Function = 15,
            Field = 16,
        };

        constexpr size_t OpKindBits = 4;
        constexpr size_t OpKindMask = (1 << OpKindBits) - 1;

        struct ILWriterOptions
        {
            bool writeDebugInfo = false;
            ILMetadata& metadata;
        };

        class ILWriter
        {
            Stream* stream;
            ILWriterOptions options;

            void Write(const void* src, size_t s)
            {
                auto result = stream->Write(src, s);
                if (result == EOF)
                {
                    throw std::runtime_error("Unexpected error encountered while writing to a stream.");
                }
            }

            void WriteByte(uint8_t value)
            {
                Write(&value, 1);
            }

            template <typename T>
            void Write(const T& value)
            {
                Write(&value, sizeof(T));
            }

            void EncodeOpCode(ILOpCode code);
            void EncodeVarId(ILVarId varId);
            void EncodeOperand(const Operand* op);
            void EncodeImmediate(const Number& imm);

        public:
            ILWriter(Stream* stream, const ILWriterOptions& options) : stream(stream), options(options) {}

            void Write(const Instruction& instr);
        };

        struct ILReaderOptions
        {
            BumpAllocator& allocator;
            ILMetadata& metadata;
            bool readDebugInfo = false;
        };

        class ILReader
        {
            Stream* stream;
            ILReaderOptions options;

            void Read(void* dst, size_t s)
            {
                auto read = stream->Read(dst, s);
                if (read != s)
                {
                    throw std::runtime_error("Unexpected end of stream.");
                }
            }

            uint8_t ReadByte()
            {
                uint8_t val;
                Read(&val, 1);
                return val;
            }

            template <typename T>
            T Read()
            {
                T dst{};
                Read(&dst, sizeof(T));
                return dst;
            }

        ILOpCode DecodeOpCode();
        ILVarId DecodeVarId();
        Operand* DecodeOperand(OpKind kind);

    public:
        ILReader(Stream* stream, const ILReaderOptions& options) : stream(stream), options(options) {}

            Instruction* Read();
        };
    }
}

#endif