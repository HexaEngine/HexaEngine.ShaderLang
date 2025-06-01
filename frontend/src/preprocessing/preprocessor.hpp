#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include "lexical/token_stream.hpp"
#include "lexical/text_mapping.hpp"
#include "parsers/parser.hpp"
#include "utils/span.hpp"

namespace HXSL
{
	struct TokenCollection
	{
		std::vector<Token> tokens;

		void AddToken(const Token& token)
		{
			tokens.push_back(token);
		}

		operator std::vector<Token>& ()
		{
			return tokens;
		}
	};

	struct MacroSymbol : public TokenCollection
	{
		std::unique_ptr<std::string> name;
		std::vector<std::unique_ptr<std::string>> parameters;
		std::unordered_map<StringSpan, size_t, StringSpanHash, StringSpanEqual> parametersLookup;

		MacroSymbol(TextSpan span) : name(std::make_unique<std::string>(span.str()))
		{
		}

		void AddParameter(const TextSpan& span)
		{
			auto index = parameters.size();
			parameters.push_back(std::make_unique<std::string>(span.str()));
			parametersLookup.insert({ StringSpan(*parameters.back()), index });
		}
	};

	struct TokenExpression : public TokenCollection
	{
	};

	struct PreprocessorState
	{
		SourceFile* file = nullptr;
		uint32_t linesSkippedCumulative = 0;
		uint32_t columnsSkippedCumulative = 0;
		size_t includeEnd = SIZE_MAX;

		TextMapping MakeMapping(size_t start, size_t end, int32_t lineOffset, int32_t columnOffset, bool resetColumn)
		{
			linesSkippedCumulative += lineOffset;
			columnsSkippedCumulative = lineOffset == 0 && !resetColumn ? columnsSkippedCumulative + columnOffset : columnOffset;
			return TextMapping(file, start, end - start, linesSkippedCumulative, columnsSkippedCumulative);
		}
	};

	struct OffsetMapping
	{
		char deltaNext;
		char deltaOffset;
	};

	class Encoder
	{
		size_t last = 0;
		std::vector<uint8_t>& storage;

	public:
		Encoder(std::vector<uint8_t>& storage) : storage(storage)
		{
		}

		void Encode(uint64_t offset)
		{
			int64_t delta = static_cast<int64_t>(offset) - static_cast<int64_t>(last);
			uint64_t zigzag = static_cast<uint64_t>((delta >> 63) ^ (delta << 1));

			do
			{
				uint8_t b = zigzag & 0x7F;
				zigzag >>= 7;
				b |= (zigzag != 0) << 7;
				storage.push_back(b);
			} while (zigzag);

			last = offset;
		}
	};

	struct TokenWriter
	{
		TextStream& stream;
		TokenWriter(TextStream& stream) : stream(stream) {}

		const void AddToken(const Token& token)
		{
			stream.Write(token.Span.span());
		}
	};

	class Decoder
	{
		uint64_t last = 0;
		size_t index = 0;
		std::vector<uint8_t>& storage;

	public:
		Decoder(std::vector<uint8_t>& storage) : storage(storage)
		{
		}

		uint64_t Decode()
		{
			uint8_t b;
			uint64_t zigzag = 0;
			size_t shift = 0;
			do
			{
				b = storage[index++];
				zigzag |= (static_cast<uint64_t>(b & 0x7F)) << shift;
				shift += 7;
			} while (b & 0x80);

			int64_t delta = static_cast<int64_t>(zigzag >> 1) ^ -static_cast<int64_t>(zigzag & 1);
			uint64_t offset = static_cast<uint64_t>(delta + static_cast<int64_t>(last));
			last = offset;
			return offset;
		}
	};

	class OffsetMappingStorage
	{
		std::vector<uint8_t> storage;
		Encoder encoder;

	public:
		OffsetMappingStorage() : encoder(Encoder(storage))
		{
		}

		void AddOffset(uint64_t value) { encoder.Encode(value); }
	};

	enum IfState : char
	{
		IfState_None = 0,
		IfState_BranchTaken = 1 << 0
	};

	enum class PrepTransformResult
	{
		Keep = 0,
		Skip = 1,
		Loop = 2
	};

	DEFINE_FLAGS_OPERATORS(IfState, char);

	class Preprocessor
	{
		ILogger* logger;
		ASTContext& context;
		std::unordered_map<StringSpan, MacroSymbol, StringSpanHash, StringSpanEqual> symbolTable;
		std::vector<TextMapping> mappings;
		OffsetMappingStorage lineOffsets;
		std::stack<PreprocessorState> stack;
		PreprocessorState state;
		size_t lastIndex = 0;
		std::unique_ptr<TextStream> outputStream;
		IfState ifState = IfState_None;
		std::stack<IfState> ifStateStack;
		std::vector<DiagnosticSuppressionRange> suppressionRanges;

		void ParseMacroExpression(TokenStream& stream, Parser& parser, TokenCollection& tokens);

		template<typename TokenOutput>
		void ExpandMacroInner(TokenStream& stream, Parser& parser, MacroSymbol& symbol, TokenOutput* output)
		{
			std::vector<TokenExpression> args;
			if (symbol.parameters.size() > 0)
			{
				if (stream.TryGetDelimiter('('))
				{
					bool first = true;

					while (!stream.TryGetDelimiter(')'))
					{
						if (!first)
						{
							if (!stream.ExpectDelimiter(',', EXPECTED_COMMA))
							{
								if (!parser.TryRecoverParameterListMacro(false))
								{
									break;
								}
								continue;
							}
							stream.SkipWhitespacesOnce();
						}
						first = false;

						size_t parenthesesDepth = 0;
						TokenExpression expr;

						while (stream.CanAdvance())
						{
							Token token = stream.Current();

							if (token.isDelimiterOf('('))
							{
								parenthesesDepth++;
							}
							else if (token.isDelimiterOf(')'))
							{
								if (parenthesesDepth == 0)
								{
									break;
								}
								parenthesesDepth--;
							}
							else if (token.isDelimiterOf(','))
							{
								if (parenthesesDepth == 0)
								{
									break;
								}
								stream.LogFormatted(UNEXPECTED_TOKEN);
								stream.Advance();
								continue;
							}

							if (token.isIdentifier())
							{
								auto span = token.Span.span();
								auto it = symbolTable.find(span);
								if (it != symbolTable.end())
								{
									stream.Advance();
									ExpandMacroInner(stream, parser, it->second, &expr);
									continue;
								}

								if (span == "defined")
								{
									stream.Advance();
									stream.ExpectDelimiter('(', EXPECTED_LEFT_PAREN);
									TextSpan name;
									stream.ExpectIdentifier(name);
									stream.ExpectDelimiter(')', EXPECTED_RIGHT_PAREN);
									bool isDefined = symbolTable.find(name.span()) != symbolTable.end();
									expr.AddToken(Token(name, TokenType_Numeric, Number(isDefined)));
									continue;
								}
							}
							expr.AddToken(stream.Current());

							stream.Advance();
						}
						args.push_back(expr);
					}
				}
			}

			if (args.size() != symbol.parameters.size())
			{
				stream.LogFormatted(MACRO_PARAM_COUNT_MISMATCH);
			}

			size_t paramIndex = 0;
			for (auto& t : symbol.tokens)
			{
				auto itt = symbol.parametersLookup.find(t.Span.span());
				if (itt != symbol.parametersLookup.end() && itt->second < args.size())
				{
					auto& inner = args[itt->second];
					for (auto& tInner : inner.tokens)
					{
						output->AddToken(tInner);
					}
				}
				else
				{
					output->AddToken(t);
				}
			}
		}

		PrepTransformResult TryExpandMacro(TokenStream& stream, Parser& parser, const Token& current);

		Number EvalExpression(TokenStream& stream, Parser& parser);

		PrepTransformResult SkipPreprocessorBlock(TokenStream& stream, Parser& parser);

		PrepTransformResult HandleIfdef(TokenStream& stream, Parser& parser, bool negate);

		void MakeMapping(size_t start, size_t end, int32_t lineOffset, int32_t columnOffset, bool resetColumn = false);

	public:
		Preprocessor(ILogger* logger, ASTContext& context) : logger(logger), context(context), outputStream(std::make_unique<TextStream>())
		{
		}

		void Process(SourceFile* file);

		PrepTransformResult Transform(Token& current, TokenStream& stream, Parser& parser);
	};
}

#endif