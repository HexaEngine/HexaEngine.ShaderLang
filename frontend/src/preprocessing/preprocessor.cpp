#include "preprocessor.hpp"
#include "pch/localization.hpp"
#include "parsers/parser.hpp"
#include "parsers/hybrid_expr_parser.hpp"
#include "evaluator.hpp"
namespace HXSL
{
	void Preprocessor::ParseMacroExpression(TokenStream& stream, Parser& parser, TokenCollection& tokens)
	{
		while (!stream.Current().isNewLine() && stream.CanAdvance())
		{
			auto current = stream.Current();
			if (current.isIdentifier())
			{
				auto span = current.Span.span();
				auto it = symbolTable.find(span);
				if (it != symbolTable.end())
				{
					stream.Advance();
					ExpandMacroInner(stream, parser, it->second, &tokens);
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
					tokens.AddToken(Token(name, TokenType_Numeric, Number(isDefined)));
					continue;
				}
			}

			tokens.AddToken(current);
			stream.Advance();
		}
	}

	PrepTransformResult Preprocessor::TryExpandMacro(TokenStream& stream, Parser& parser, const Token& current)
	{
		auto id = current.Span.span();
		auto it = symbolTable.find(id);
		if (it == symbolTable.end())
		{
			return PrepTransformResult::Keep;
		}

		auto start = outputStream->GetPosition();
		stream.SkipWhitespace(false);
		auto writer = TokenWriter(*outputStream.get());
		ExpandMacroInner(stream, parser, it->second, &writer);

		auto end = outputStream->GetPosition();

		MakeMapping(start, end, 0, -1, true);

		return PrepTransformResult::Keep;
	}

	Number Preprocessor::EvalExpression(TokenStream& stream, Parser& parser)
	{
		TokenCollection tokens;
		ParseMacroExpression(stream, parser, tokens);
		Evaluator eval;
		return eval.evaluate(tokens);
	}

	PrepTransformResult Preprocessor::SkipPreprocessorBlock(TokenStream& stream, Parser& parser)
	{
		bool found = false;
		size_t depth = 1;
		do
		{
			auto current = stream.Current();
			if (current.isKeyword())
			{
				auto keyword = current.asKeyword();
				switch (keyword)
				{
				case Keyword_PrepIf:
					depth++;
					break;
				case Keyword_PrepElif:
					if (depth == 1)
					{
						found = true;
					}
					break;
				case Keyword_PrepElse:
					if (depth == 1)
					{
						found = true;
					}
					break;
				case Keyword_PrepEndif:
					depth--;
					if (depth == 0)
					{
						found = true;
					}
					break;
				}
			}
			else if (current.isDelimiterOf('#'))
			{
				stream.LogFormatted(EXPECTED_PREP_DIRECTIVE);
			}
			else if (current.isNewLine())
			{
				outputStream->Write("\n", 1);
			}

			if (found)
			{
				break;
			}
		} while (stream.TryAdvance());

		if (found)
		{
			return PrepTransformResult::Loop;
		}
		else
		{
			stream.LogFormatted(UNEXPECTED_EOS);
		}

		return PrepTransformResult::Keep;
	}

	PrepTransformResult Preprocessor::HandleIfdef(TokenStream& stream, Parser& parser, bool negate)
	{
		stream.SkipWhitespace(true);
		stream.Advance();
		TextSpan name;
		stream.ExpectIdentifier(name);

		bool result = (symbolTable.find(name.span()) != symbolTable.end()) ^ negate;

		ifStateStack.push(ifState);
		ifState = IfState_None;
		if (result)
		{
			ifState |= IfState_BranchTaken;
		}
		else
		{
			return SkipPreprocessorBlock(stream, parser);
		}
		return PrepTransformResult::Skip;
	}

	void Preprocessor::MakeMapping(size_t start, size_t end, int32_t lineOffset, int32_t columnOffset, bool resetColumn)
	{
		if (!mappings.empty())
		{
			auto& last = mappings.back();
			if (last.file == state.file && last.start == start)
			{
				last.columnOffset += columnOffset;
				last.lineOffset += lineOffset;
				state.columnsSkippedCumulative += columnOffset;
				state.linesSkippedCumulative += lineOffset;
				return;
			}
		}

		mappings.push_back(state.MakeMapping(start, end, lineOffset, columnOffset, resetColumn));
	}

	static inline bool StartsWith(const char* current, const char* end, const std::string& match)
	{
		return current + match.size() < end && std::memcmp(current, match.c_str(), match.size()) == 0;
	}

	struct PrepTokenStream : TokenStream
	{
		TextStream* outputStream;

		PrepTokenStream(LexerContext* context, TextStream* outputStream) : TokenStream(context), outputStream(outputStream)
		{
		}

		bool TryAdvance() override
		{
			auto& state = streamState;
			auto& lexerState = state.state;
			auto& currentToken = state.currentToken;
			auto& cache = this->cache;

			state.lastToken = currentToken;

			/*
			if (cache.CanAccess(state.tokenPosition))
			{
				if (currentStack == 0)
				{
					currentToken = cache.GetToken();
				}
				else
				{
					currentToken = cache.PeekToken(state.tokenPosition);
				}

				state.tokenPosition++;
				return true;
			}
			*/

			do
			{
				if (IsEndOfTokens() || context->HasCriticalErrors())
				{
					currentToken = {};
					return false;
				}

				currentToken = Lexer::TokenizeStep(lexerState);
				lexerState.Advance();

				if (currentToken.Type == TokenType_Comment)
				{
					auto diff = lexerState.Line - currentToken.Span.line;
					for (size_t i = 0; i < diff; i++)
					{
						outputStream->Write("\n", 1);
					}
				}
			} while (currentToken.Type == TokenType_Comment || currentToken.Type == TokenType_Unknown || (skipWhitespace && currentToken.Type == TokenType_Whitespace));

			/*
			if (currentStack == 0)
			{
				cache.IncrementPosition();
			}
			else
			{
				cache.AddToken(currentToken);
			}
			*/

			state.tokenPosition++;
			return true;
		}
	};

	void Preprocessor::Process(SourceFile* file)
	{
		state.file = file;
		LexerContext lexerContext = LexerContext(ASTContext::GetCurrentContext()->GetIdentifierTable(), file, file->GetInputStream().get(), logger, HXSLLexerConfig::InstancePreprocess());
		PrepTokenStream stream = PrepTokenStream(&lexerContext, outputStream.get());
		Parser parser = Parser(logger, stream);
		auto result = PrepTransformResult::Keep;
		while (stream.CanAdvance())
		{
			stream.Advance();
			do
			{
				auto current = stream.Current();
				result = Transform(current, stream, parser);
				current = stream.Current();
				stream.SkipWhitespace(false);

				while (result == PrepTransformResult::Keep)
				{
					if (current.isNewLine())
					{
						outputStream->Write("\n", 1);
						break;
					}

					outputStream->Write(current.Span.span());
					break;
				}
			} while (result == PrepTransformResult::Loop);
		}

		file->SetInputStream(std::move(outputStream));
	}

	PrepTransformResult Preprocessor::Transform(Token& current, TokenStream& stream, Parser& parser)
	{
		if (current.isNewLine())
		{
			return PrepTransformResult::Keep;
		}

		auto& lexerState = stream.GetLexerState();

		if (lexerState.Index >= state.includeEnd)
		{
			state = stack.top();
			stack.pop();
		}

		if (current.isDelimiterOf('#'))
		{
			stream.LogFormatted(EXPECTED_PREP_DIRECTIVE);
			stream.Advance();
			return PrepTransformResult::Keep;
		}

		if (current.isIdentifier())
		{
			stream.SkipWhitespace(true);
			return TryExpandMacro(stream, parser, current);
		}

		if (!current.isKeyword())
		{
			return PrepTransformResult::Keep;
		}

		auto keyword = current.asKeyword();

		switch (keyword)
		{
		case Keyword_PrepDefine:
		{
			stream.SkipWhitespace(true);
			stream.Advance();
			TextSpan name;
			stream.ExpectIdentifier(name);
			MacroSymbol symbol = MacroSymbol(name);

			if (stream.TryGetDelimiter('('))
			{
				bool first = true;
				while (!stream.TryGetDelimiter(')'))
				{
					if (!first)
					{
						if (!stream.ExpectDelimiter(',', EXPECTED_COMMA))
						{
							if (!parser.TryRecoverParameterListMacro(true))
							{
								break;
							}
							continue;
						}
					}
					first = false;
					TextSpan paramName;
					stream.ExpectIdentifier(paramName, EXPECTED_IDENTIFIER);
					symbol.AddParameter(paramName);
				}
			}

			stream.SkipWhitespace(false);
			ParseMacroExpression(stream, parser, symbol);

			StringSpan nameSpan = *symbol.name.get();
			symbolTable.insert(std::make_pair(nameSpan, std::move(symbol)));
		}
		break;
		case Keyword_PrepIf:
		{
			stream.SkipWhitespace(true);
			stream.Advance();
			auto result = EvalExpression(stream, parser);

			ifStateStack.push(ifState);
			ifState = IfState_None;
			if (result.ToBool())
			{
				ifState |= IfState_BranchTaken;
			}
			else
			{
				return SkipPreprocessorBlock(stream, parser);
			}
		}
		break;
		case Keyword_PrepElif:
		{
			stream.SkipWhitespace(true);
			stream.Advance();
			if (ifStateStack.empty())
			{
				stream.LogFormatted(PREP_MISSING_IF);
				break;
			}
			else if ((ifState & IfState_BranchTaken) != 0)
			{
				return SkipPreprocessorBlock(stream, parser);
			}

			auto result = EvalExpression(stream, parser);
			if (result.ToBool())
			{
				ifState |= IfState_BranchTaken;
			}
			else
			{
				return SkipPreprocessorBlock(stream, parser);
			}
		}
		break;
		case Keyword_PrepElse:
		{
			stream.SkipWhitespace(true);
			stream.TryAdvance();
			if (ifStateStack.empty())
			{
				stream.LogFormatted(PREP_MISSING_IF);
				return PrepTransformResult::Keep;
			}
			else if ((ifState & IfState_BranchTaken) != 0)
			{
				return SkipPreprocessorBlock(stream, parser);
			}
		}
		break;
		case Keyword_PrepEndif:
		{
			stream.SkipWhitespace(true);
			stream.TryAdvance();
			if (ifStateStack.empty())
			{
				stream.LogFormatted(PREP_MISSING_IF);
				return PrepTransformResult::Keep;
			}
			ifState = ifStateStack.top();
			ifStateStack.pop();
		}
		break;
		case Keyword_PrepIfdef: return HandleIfdef(stream, parser, false);
		case Keyword_PrepIfndef: return HandleIfdef(stream, parser, true);
		case Keyword_PrepInclude:
		{
			TextSpan literal;
			stream.ExpectLiteral(literal);
		}
		break;
		case Keyword_PrepError:
		{
		}
		break;
		case Keyword_PrepWarning:
		{
		}
		break;
		case Keyword_PrepPragma:
		{
			stream.SkipWhitespace(true);
			stream.Advance();

			TextSpan literal;
			stream.ExpectIdentifier(literal);
			auto span = literal.span();

			if (span == "warning")
			{
				TextSpan op;
				stream.ExpectIdentifier(op);
				auto opSpan = op.span();

				TextSpan code;
				stream.ExpectIdentifier(code);

				auto diagCode = DiagnosticCode::Encode(LogLevel_Warn, code.str());

				if (opSpan == "disable")
				{
					suppressionRanges.push_back(DiagnosticSuppressionRange(diagCode, outputStream->GetPosition(), -1));
				}
				else if (opSpan == "restore")
				{
					for (size_t i = 0; i < suppressionRanges.size(); i++)
					{
						auto& supr = suppressionRanges[i];
						if (supr.code == diagCode)
						{
							supr.end = outputStream->GetPosition();
							logger->AddDiagnosticSuppressionRange(supr);
							suppressionRanges.erase(suppressionRanges.begin() + i);
						}
					}
				}
			}
		}
		break;
		}

		return PrepTransformResult::Keep;
	}
}