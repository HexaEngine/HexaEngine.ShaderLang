#ifndef LEXER_HPP
#define LEXER_HPP

#include "utils/text_helper.hpp"
#include "io/source_file.hpp"
#include "logging/logger.hpp"
#include "logging/diagnostic_code.hpp"
#include "lang/language.hpp"
#include "token.hpp"
#include "lexer_config.hpp"
#include "input_stream.hpp"

#include "pch/std.hpp"

namespace HXSL
{
	struct LexerState;

	class LexerContext
	{
	public:
		SourceFile* source;
		LexerStream* stream;
		ILogger* logger;
		const LexerConfig* config;

		LexerContext(SourceFile* source, LexerStream* stream, ILogger* logger, const LexerConfig* config) : source(source), stream(stream), logger(logger), config(config)
		{
		}

		LexerState MakeState();

		const char* GetBuffer() const noexcept { return stream->GetBuffer(); }

		size_t GetLength() const noexcept { return stream->GetLength(); }

		bool HasCriticalErrors() const noexcept
		{
			return logger->HasCriticalErrors();
		}
	};

	struct LexerState
	{
	public:
		LexerContext* context;
		size_t Index;
		size_t IndexNext;
		uint32_t Line;
		uint32_t Column;

		LexerState() : context(nullptr), Index(0), IndexNext(0), Line(1), Column(1)
		{
		}

		LexerState(LexerContext* context) : context(context), Index(0), IndexNext(0), Line(1), Column(1)
		{
		}

		const char* GetBuffer() const noexcept { return context->GetBuffer(); }

		size_t GetLength() const noexcept { return context->GetLength(); }

		const LexerConfig* GetConfig() const noexcept { return context->config; }

		template <typename... Args>
		void LogFormatted(DiagnosticCode code, Args&&... args) const
		{
			context->logger->LogFormattedEx(code, Index, " (Line: {}, Column: {})", std::forward<Args>(args)..., Line, Column);
		}

		const char* Current() const noexcept
		{
			return GetBuffer() + Index;
		}

		const char* End() const noexcept
		{
			return GetBuffer() + GetLength();
		}

		bool IsEOF() const noexcept
		{
			return Index >= GetLength();
		}

		void Advance()
		{
			size_t diff = IndexNext - Index;
			Index = IndexNext;
			Column += static_cast<uint32_t>(diff);
		}

		void Jump(size_t index)
		{
			HXSL_ASSERT(Index < index, "Jump index must be larger than the current.");
			IndexNext = Index = index;
		}

		void NewLine(uint32_t lines = 1)
		{
			Line += lines;
			Column = 1;
		}

		bool MatchPair(char current, char first, char second) const
		{
			return TextHelper::MatchPair(GetBuffer(), Index, GetLength(), current, first, second);
		}

		size_t FindEndOfLine(size_t start) const
		{
			return TextHelper::FindEndOfLine(GetBuffer(), start, GetLength());
		}

		size_t FindWordBoundary(size_t start) const
		{
			return TextHelper::FindWordBoundary(GetBuffer(), start, GetLength());
		}

		size_t FindOperatorBoundary(size_t start, const std::unordered_set<char>& delimiters) const
		{
			return TextHelper::FindOperatorBoundary(GetBuffer(), start, GetLength(), delimiters);
		}

		bool LookAhead(size_t start, char target, size_t& trackedLength, size_t& lines) const
		{
			return TextHelper::LookAhead(GetBuffer(), start, GetLength(), target, trackedLength, lines);
		}

		bool LookAhead(size_t start, const std::string& target, size_t& trackedLength, size_t& lines) const
		{
			return TextHelper::LookAhead(GetBuffer(), start, GetLength(), target, trackedLength, lines);
		}

		bool TryParseIdentifier(size_t& trackedLength) const
		{
			return TextHelper::TryParseIdentifier(GetBuffer(), Index, GetLength(), trackedLength);
		}

		TextSpan AsTextSpan(size_t start, size_t length) const
		{
			return TextSpan(context->source, start, length, Line, Column);
		}

		TextSpan AsTextSpan() const
		{
			return TextSpan(context->source, Index, GetLength() - Index, Line, Column);
		}

		StringSpan AsSpan() const
		{
			return StringSpan(GetBuffer(), Index, GetLength() - Index);
		}
	};

	class Lexer
	{
	public:
		static Token TokenizeStep(LexerState& state);
	};
}

#endif