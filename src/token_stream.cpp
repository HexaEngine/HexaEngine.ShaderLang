#include "token_stream.h"

namespace HXSL
{
	void TokenStream::PushState()
	{
		stack.push(StreamState);
		currentStack++;
	}

	void TokenStream::PopState(bool restore)
	{
		currentStack--;
		if (restore)
		{
			StreamState = stack.top();
		}
		else if (currentStack == 0)
		{
			auto& state = StreamState;
			auto& cache = Cache;
			if (state.TokenPosition >= cache.Position())
			{
				cache.Flush(state.TokenPosition);
			}
		}
		stack.pop();
	}

	void TokenStream::Advance()
	{
		if (!TryAdvance())
		{
			if (IsEndOfTokens())
			{
				StreamState.State.LogError("Unexpected end of stream.");
			}
		}
	}

	bool TokenStream::TryAdvance()
	{
		auto& state = StreamState;
		auto& lexerState = state.State;
		auto& currentToken = state.CurrentToken;
		auto& cache = Cache;

		state.LastToken = currentToken;

		if (cache.CanAccess(state.TokenPosition))
		{
			if (currentStack == 0)
			{
				currentToken = cache.GetToken();
			}
			else
			{
				currentToken = cache.PeekToken(state.TokenPosition);
			}

			state.TokenPosition++;
			return true;
		}

		do
		{
			if (IsEndOfTokens() || lexerState.HasCriticalErrors())
			{
				currentToken = {};
				return false;
			}

			currentToken = Lexer::TokenizeStep(lexerState, Config);
			lexerState.Advance();
		} while (currentToken.Type == TokenType_Comment);

		if (currentStack == 0)
		{
			cache.IncrementPosition();
		}
		else
		{
			cache.AddToken(currentToken);
		}

		state.TokenPosition++;
		return true;
	}
}