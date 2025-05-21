#include "diagnostic_code.hpp"
#include "pch/localization.hpp"

namespace HXSL
{
	std::string DiagnosticCode::GetMessage() const
	{
		return GetMessageForCode(value);
	}

	std::string DiagnosticCode::GetCodeString() const
	{
		return GetStringForCode(value);
	}

	DiagnosticCode DiagnosticCode::Encode(LogLevel level, const std::string& codeId)
	{
		return EncodeCodeId(level, codeId);
	}
}