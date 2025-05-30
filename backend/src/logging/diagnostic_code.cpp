#include "logging/diagnostic_code.hpp"

namespace HXSL
{
	DiagnosticCode::GetMessageForCode DiagnosticCode::getMessageForCode;
	DiagnosticCode::GetStringForCode DiagnosticCode::getStringForCode;
	DiagnosticCode::EncodeDiagnosticCode DiagnosticCode::encodeDiagnosticCode;

	std::string DiagnosticCode::GetMessage() const
	{
		return getMessageForCode(value);
	}

	std::string DiagnosticCode::GetCodeString() const
	{
		return getStringForCode(value);
	}

	DiagnosticCode DiagnosticCode::Encode(LogLevel level, const std::string& codeId)
	{
		return encodeDiagnosticCode(level, codeId);
	}
}