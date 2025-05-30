#include "lexical/text_span.hpp"

namespace HXSL
{
	TextSpan::TextSpanGetStr TextSpan::textSpanGetStr;
	TextSpan::TextSpanGetSpan TextSpan::textSpanGetSpan;

	std::string HXSL::TextSpan::str() const
	{
		return textSpanGetStr(*this);
	}

	StringSpan TextSpan::span() const
	{
		return textSpanGetSpan(*this);
	}
}