#include "token_stream.h"
#include <generated/localization.hpp>

namespace HXSL
{
	void TokenStream::PushState()
	{
		stack.push(streamState);
		currentStack++;
	}

	void TokenStream::PopState(bool restore)
	{
		currentStack--;
		if (restore)
		{
			streamState = stack.top();
		}
		/*
		else if (currentStack == 0)
		{
			auto& state = streamState;
			auto& cache = this->cache;
			if (state.tokenPosition >= cache.Position())
			{
				cache.Flush(state.tokenPosition);
			}
		}*/
		stack.pop();
	}

	void TokenStream::Advance()
	{
		if (!TryAdvance())
		{
			if (IsEndOfTokens())
			{
				LogFormatted(UNEXPECTED_EOS, "");
			}
		}
	}

	bool TokenStream::TryAdvance()
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

		int result = 0;
		do
		{
			do
			{
				if (IsEndOfTokens() || context->HasCriticalErrors())
				{
					currentToken = {};
					return false;
				}

				currentToken = TokenizeStep(lexerState);
				lexerState.Advance();
			} while (currentToken.Type == TokenType_Comment || currentToken.Type == TokenType_Unknown);

			if (!transforming)
			{
				transforming = true;
				Token out;
				result = transformer.Transform(currentToken, *this);
				transforming = false;
			}
		} while (result == 2);

		if (currentStack == 0)
		{
			cache.IncrementPosition();
		}
		else
		{
			cache.AddToken(currentToken);
		}

		state.tokenPosition++;
		return true;
	}
}