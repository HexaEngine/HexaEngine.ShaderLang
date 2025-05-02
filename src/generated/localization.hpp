#ifndef LOCALIZATION_HPP
#define LOCALIZATION_HPP

#include <memory>
#include <unordered_map>
#include <string>
#include "diagnostic_code.hpp"
#include "io/log.h"

// Automatically generated locale codes

namespace HXSL
{
	extern std::unique_ptr<std::unordered_map<uint64_t, std::string>> current_locale_map;
	
	extern void SetLocale(const std::string& language_code);
	extern std::string GetMessageForCode(uint64_t code);
	extern std::string GetStringForCode(uint64_t code);
	extern LogLevel GetLogLevelForCode(uint64_t code);
	extern uint64_t EncodeCodeId(LogLevel level, const std::string& input);
	
	/// <summary>
	/// <para>Code: HL0001</para>
	/// <para>Message: invalid token</para>
	/// <para>Description: A token that is not recognized by the lexer. This often occurs due to a typo or unsupported character.</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_TOKEN = 9223373187906027521;
	
	/// <summary>
	/// <para>Code: HL0002</para>
	/// <para>Message: comment unclosed at end of file</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode MISSING_END_COMMENT = 9223373187906027522;
	
	/// <summary>
	/// <para>Code: HL0003</para>
	/// <para>Message: missing closing quote</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode MISSING_QUOTE = 9223373187906027523;
	
	/// <summary>
	/// <para>Code: HL0004</para>
	/// <para>Message: unexpected end of stream</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_EOS = 9223373187906027524;
	
	/// <summary>
	/// <para>Code: HL0020</para>
	/// <para>Message: expected ';'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_SEMICOLON = 9223373187906027540;
	
	/// <summary>
	/// <para>Code: HL0021</para>
	/// <para>Message: expected '{'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_LEFT_BRACE = 9223373187906027541;
	
	/// <summary>
	/// <para>Code: HL0022</para>
	/// <para>Message: expected '}'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_RIGHT_BRACE = 9223373187906027542;
	
	/// <summary>
	/// <para>Code: HL0023</para>
	/// <para>Message: expected '('</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_LEFT_PAREN = 9223373187906027543;
	
	/// <summary>
	/// <para>Code: HL0024</para>
	/// <para>Message: expected ')'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_RIGHT_PAREN = 9223373187906027544;
	
	/// <summary>
	/// <para>Code: HL0025</para>
	/// <para>Message: expected ','</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_COMMA = 9223373187906027545;
	
	/// <summary>
	/// <para>Code: HL0026</para>
	/// <para>Message: expected '['</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_LEFT_BRACKET = 9223373187906027546;
	
	/// <summary>
	/// <para>Code: HL0027</para>
	/// <para>Message: expected ']'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_RIGHT_BRACKET = 9223373187906027547;
	
	/// <summary>
	/// <para>Code: HL0028</para>
	/// <para>Message: expected '#'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_HASH = 9223373187906027548;
	
	/// <summary>
	/// <para>Code: HL0029</para>
	/// <para>Message: expected '@'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_AT = 9223373187906027549;
	
	/// <summary>
	/// <para>Code: HL0030</para>
	/// <para>Message: unexpected token encountered</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_TOKEN = 9223373187906027550;
	
	/// <summary>
	/// <para>Code: HL0031</para>
	/// <para>Message: expected 'namespace'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_NAMESPACE = 9223373187906027551;
	
	/// <summary>
	/// <para>Code: HL0032</para>
	/// <para>Message: expected an identifier</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_IDENTIFIER = 9223373187906027552;
	
	/// <summary>
	/// <para>Code: HL0033</para>
	/// <para>Message: expected a number</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_NUMBER = 9223373187906027553;
	
	/// <summary>
	/// <para>Code: HL0034</para>
	/// <para>Message: expected an literal</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_LITERAL = 9223373187906027554;
	
	/// <summary>
	/// <para>Code: HL0035</para>
	/// <para>Message: expected a operator</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_OPERATOR = 9223373187906027555;
	
	/// <summary>
	/// <para>Code: HL0036</para>
	/// <para>Message: expected ':'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_COLON = 9223373187906027556;
	
	/// <summary>
	/// <para>Code: HL0037</para>
	/// <para>Message: expected 'case' or 'default'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_CASE_OR_DEFAULT = 9223373187906027557;
	
	/// <summary>
	/// <para>Code: HL0050</para>
	/// <para>Message: unexpected end of expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_END_OF_EXPRESSION = 9223373187906027570;
	
	/// <summary>
	/// <para>Code: HL0100</para>
	/// <para>Message: namespaces must be at the global scope</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode NAMESPACE_MUST_BE_GLOBAL_SCOPE = 9223373187906027620;
	
	/// <summary>
	/// <para>Code: HL0101</para>
	/// <para>Message: only one namespace can be declared in the current scope</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode ONLY_ONE_NAMESPACE_ALLOWED = 9223373187906027621;
	
	/// <summary>
	/// <para>Code: HL0102</para>
	/// <para>Message: usings must be at the global or namespace scope</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode USINGS_MUST_BE_GLOBAL_OR_NAMESPACE_SCOPE = 9223373187906027622;
	
	/// <summary>
	/// <para>Code: HL0103</para>
	/// <para>Message: 'public' cannot be combined with 'private', 'protected', or 'internal'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode PUB_CANNOT_COMBINE_WITH_PRIV_PROT_INT = 9223373187906027623;
	
	/// <summary>
	/// <para>Code: HL0104</para>
	/// <para>Message: 'internal' cannot be combined with 'private' or 'public'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INT_CANNOT_COMBINE_WITH_PRIV_OR_PUB = 9223373187906027624;
	
	/// <summary>
	/// <para>Code: HL0105</para>
	/// <para>Message: 'protected' cannot be combined with 'private' or 'public'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode PROT_CANNOT_COMBINE_WITH_PRIV_OR_PUB = 9223373187906027625;
	
	/// <summary>
	/// <para>Code: HL0106</para>
	/// <para>Message: 'private' cannot be combined with 'public', 'protected', or 'internal'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode PRIV_CANNOT_COMBINE_WITH_PUB_PROT_INT = 9223373187906027626;
	
	/// <summary>
	/// <para>Code: HL0107</para>
	/// <para>Message: 'in' cannot be combined with 'out' or 'inout'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode IN_CANNOT_COMBINE_WITH_OUT_OR_INOUT = 9223373187906027627;
	
	/// <summary>
	/// <para>Code: HL0108</para>
	/// <para>Message: 'out' cannot be combined with 'in' or 'inout'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode OUT_CANNOT_COMBINE_WITH_IN_OR_INOUT = 9223373187906027628;
	
	/// <summary>
	/// <para>Code: HL0109</para>
	/// <para>Message: 'inout' cannot be combined with 'in' or 'out'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INOUT_CANNOT_COMBINE_WITH_IN_OR_OUT = 9223373187906027629;
	
	/// <summary>
	/// <para>Code: HL0110</para>
	/// <para>Message: 'linear' cannot be combined with 'centroid' or 'nointerpolation'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode LINEAR_CANNOT_COMBINE_WITH_CENTROID_OR_NOINTERP = 9223373187906027630;
	
	/// <summary>
	/// <para>Code: HL0111</para>
	/// <para>Message: 'centroid' cannot be combined with 'linear' or 'nointerpolation'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode CENTROID_CANNOT_COMBINE_WITH_LINEAR_OR_NOINTERP = 9223373187906027631;
	
	/// <summary>
	/// <para>Code: HL0112</para>
	/// <para>Message: 'nointerpolation' cannot be combined with other interpolation modifiers</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode NOINTERP_CANNOT_COMBINE_WITH_OTHER_INTERP_MODIFIERS = 9223373187906027632;
	
	/// <summary>
	/// <para>Code: HL0113</para>
	/// <para>Message: 'noperspecitve' cannot be combined with 'nointerpolation'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode NOPERSPECTIVE_CANNOT_COMBINE_WITH_NOINTERP = 9223373187906027633;
	
	/// <summary>
	/// <para>Code: HL0114</para>
	/// <para>Message: 'sample' cannot be combined with 'nointerpolation'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode SAMPLE_CANNOT_COMBINE_WITH_NOINTERP = 9223373187906027634;
	
	/// <summary>
	/// <para>Code: HL0200</para>
	/// <para>Message: cannot declare a struct in this scope, allowed scopes are 'namespace', 'struct' or 'class'.</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode STRUCT_DECL_OUT_OF_SCOPE = 9223373187906027720;
	
	/// <summary>
	/// <para>Code: HL0300</para>
	/// <para>Message: invalid postfix operator.</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_POSTFIX_OP = 9223373187906027820;
	
	/// <summary>
	/// <para>Code: HL1000</para>
	/// <para>Message: expected constant expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_CONST_EXPR = 9223373187906028520;
	
	/// <summary>
	/// <para>Code: HL1001</para>
	/// <para>Message: array sizes must be an integral number</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode ARRAY_SIZE_MUST_BE_INT = 9223373187906028521;
	
	/// <summary>
	/// <para>Code: HL1002</para>
	/// <para>Message: array sizes cannot be negative</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode ARRAY_SIZE_CANNOT_BE_NEG = 9223373187906028522;
	
	/// <summary>
	/// <para>Code: HL1003</para>
	/// <para>Message: increment/decrement must target a variable</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INC_DEC_MUST_TARGET_VAR = 9223373187906028523;
	
}
#endif // LOCALIZATION_HPP
