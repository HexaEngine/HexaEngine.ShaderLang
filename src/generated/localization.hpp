#ifndef LOCALIZATION_HPP
#define LOCALIZATION_HPP

#include <memory>
#include <unordered_map>
#include <string>
#include "diagnostic_code.hpp"
#include "io/logger.hpp"

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
	/// <para>Code: HL0038</para>
	/// <para>Message: unexpected ':' outside of a ternary expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_COLON_OUTSIDE_TERNARY = 9223373187906027558;
	
	/// <summary>
	/// <para>Code: HL0039</para>
	/// <para>Message: expected expression after ternary ':'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_EXPR_AFTER_TERNARY = 9223373187906027559;
	
	/// <summary>
	/// <para>Code: HL0040</para>
	/// <para>Message: expected ':' to separate branches of ternary expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_COLON_TERNARY = 9223373187906027560;
	
	/// <summary>
	/// <para>Code: HL0041</para>
	/// <para>Message: expected an operand after operator</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_OPERAND_AFTER_OP = 9223373187906027561;
	
	/// <summary>
	/// <para>Code: HL0042</para>
	/// <para>Message: expected an assignment operator</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_ASSIGNMENT_OP = 9223373187906027562;
	
	/// <summary>
	/// <para>Code: HL0043</para>
	/// <para>Message: expected '=' or ';' in a declaration expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_EQUALS_OR_SEMICOLON_DECL = 9223373187906027563;
	
	/// <summary>
	/// <para>Code: HL0050</para>
	/// <para>Message: unexpected end of expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_END_OF_EXPRESSION = 9223373187906027570;
	
	/// <summary>
	/// <para>Code: HL0051</para>
	/// <para>Message: unexpected end of expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Syntax Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_EXPRESSION = 9223373187906027571;
	
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
	/// <para>Message: cannot declare a struct in this scope</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode STRUCT_DECL_OUT_OF_SCOPE = 9223373187906027720;
	
	/// <summary>
	/// <para>Code: HL0201</para>
	/// <para>Message: cannot declare field in this scope</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode FIELD_DECL_OUT_OF_SCOPE = 9223373187906027721;
	
	/// <summary>
	/// <para>Code: HL0202</para>
	/// <para>Message: 'break' statement used outside of a loop or switch context</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_BREAK_STATEMENT = 9223373187906027722;
	
	/// <summary>
	/// <para>Code: HL0203</para>
	/// <para>Message: 'continue' statement used outside of a loop context</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_CONTINUE_STATEMENT = 9223373187906027723;
	
	/// <summary>
	/// <para>Code: HL0204</para>
	/// <para>Message: 'discard' statement used outside of a function scope</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode UNEXPECTED_DISCARD_STATEMENT = 9223373187906027724;
	
	/// <summary>
	/// <para>Code: HL0205</para>
	/// <para>Message: cannot declare two default cases in a switch-case</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode DUPLICATE_DEFAULT_CASE = 9223373187906027725;
	
	/// <summary>
	/// <para>Code: HL0206</para>
	/// <para>Message: attribute '%s' is not allowed in this context</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode ATTRIBUTE_INVALID_IN_CONTEXT = 9223373187906027726;
	
	/// <summary>
	/// <para>Code: HL0207</para>
	/// <para>Message: no modifiers are allowed in this context</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode NO_MODIFIER_INVALID_IN_CONTEXT = 9223373187906027727;
	
	/// <summary>
	/// <para>Code: HL0208</para>
	/// <para>Message: specified modifier is not allowed on variables</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_MODIFIER_ON_VAR = 9223373187906027728;
	
	/// <summary>
	/// <para>Code: HL0209</para>
	/// <para>Message: specified modifier is not allowed on fields</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_MODIFIER_ON_FIELD = 9223373187906027729;
	
	/// <summary>
	/// <para>Code: HL0210</para>
	/// <para>Message: specified modifier is not allowed on functions</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_MODIFIER_ON_FUNC = 9223373187906027730;
	
	/// <summary>
	/// <para>Code: HL0211</para>
	/// <para>Message: specified modifier is not allowed on operators</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_MODIFIER_ON_OP = 9223373187906027731;
	
	/// <summary>
	/// <para>Code: HL0212</para>
	/// <para>Message: specified modifier is not allowed on structs</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_MODIFIER_ON_STRUCT = 9223373187906027732;
	
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
	/// <para>Message: expected an type in cast expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_TYPE_EXPR_CAST = 9223373187906028521;
	
	/// <summary>
	/// <para>Code: HL1002</para>
	/// <para>Message: expected type expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_TYPE_EXPR = 9223373187906028522;
	
	/// <summary>
	/// <para>Code: HL1003</para>
	/// <para>Message: expected function call expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_FUNC_CALL_EXPR = 9223373187906028523;
	
	/// <summary>
	/// <para>Code: HL1004</para>
	/// <para>Message: array sizes must be an integral number</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode ARRAY_SIZE_MUST_BE_INT = 9223373187906028524;
	
	/// <summary>
	/// <para>Code: HL1005</para>
	/// <para>Message: array sizes cannot be negative</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode ARRAY_SIZE_CANNOT_BE_NEG = 9223373187906028525;
	
	/// <summary>
	/// <para>Code: HL1006</para>
	/// <para>Message: increment/decrement must target a variable</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INC_DEC_MUST_TARGET_VAR = 9223373187906028526;
	
	/// <summary>
	/// <para>Code: HL1007</para>
	/// <para>Message: expected boolean expression</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_BOOL_EXPR = 9223373187906028527;
	
	/// <summary>
	/// <para>Code: HL1008</para>
	/// <para>Message: operand types are not compatible '%s' and 's%'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode OPERAND_TYPES_INCOMPATIBLE = 9223373187906028528;
	
	/// <summary>
	/// <para>Code: HL1009</para>
	/// <para>Message: no suitable conversion exists to convert from '%s' to 's%'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode TYPE_CONVERSION_NOT_FOUND = 9223373187906028529;
	
	/// <summary>
	/// <para>Code: HL1010</para>
	/// <para>Message: expression must be mutable</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPR_MUST_BE_MUTABLE = 9223373187906028530;
	
	/// <summary>
	/// <para>Code: HL1011</para>
	/// <para>Message: expression must be integral type</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPR_MUST_BE_INTEGRAL = 9223373187906028531;
	
	/// <summary>
	/// <para>Code: HL1012</para>
	/// <para>Message: expression must be array type</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPR_MUST_BE_ARRAY = 9223373187906028532;
	
	/// <summary>
	/// <para>Code: HL1013</para>
	/// <para>Message: return value type '%s' does not match the function return type '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode RETURN_TYPE_DOES_NOT_MATCH = 9223373187906028533;
	
	/// <summary>
	/// <para>Code: HL1100</para>
	/// <para>Message: '%s' is already defined in this scope</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode SYMBOL_REDEFINITION = 9223373187906028620;
	
	/// <summary>
	/// <para>Code: HL1101</para>
	/// <para>Message: '%s' is not declared in the current context</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode SYMBOL_NOT_FOUND = 9223373187906028621;
	
	/// <summary>
	/// <para>Code: HL1102</para>
	/// <para>Message: the namespace '%s' couldn't be found</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode NAMESPACE_NOT_FOUND = 9223373187906028622;
	
	/// <summary>
	/// <para>Code: HL1103</para>
	/// <para>Message: the namespace or type '%s' couldn't be found</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode NAMESPACE_OR_TYPE_NOT_FOUND = 9223373187906028623;
	
	/// <summary>
	/// <para>Code: HL1104</para>
	/// <para>Message: the namespace or type '%s' couldn't be found in '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode NAMESPACE_OR_TYPE_NOT_FOUND_IN = 9223373187906028624;
	
	/// <summary>
	/// <para>Code: HL1105</para>
	/// <para>Message: the type '%s' couldn't be found</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode TYPE_NOT_FOUND = 9223373187906028625;
	
	/// <summary>
	/// <para>Code: HL1106</para>
	/// <para>Message: the type '%s' couldn't be found in '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode TYPE_NOT_FOUND_IN = 9223373187906028626;
	
	/// <summary>
	/// <para>Code: HL1107</para>
	/// <para>Message: the function '%s' couldn't be found</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode FUNC_NOT_FOUND = 9223373187906028627;
	
	/// <summary>
	/// <para>Code: HL1108</para>
	/// <para>Message: the function '%s' couldn't be found in '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode FUNC_NOT_FOUND_IN = 9223373187906028628;
	
	/// <summary>
	/// <para>Code: HL1109</para>
	/// <para>Message: the member '%s' couldn't be found</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode MEMBER_NOT_FOUND = 9223373187906028629;
	
	/// <summary>
	/// <para>Code: HL1110</para>
	/// <para>Message: the member '%s' couldn't be found in '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode MEMBER_NOT_FOUND_IN = 9223373187906028630;
	
	/// <summary>
	/// <para>Code: HL1111</para>
	/// <para>Message: cannot cast from '%s' to '%s' no cast operator defined</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode CANNOT_CAST_FROM_TO = 9223373187906028631;
	
	/// <summary>
	/// <para>Code: HL1112</para>
	/// <para>Message: cannot overload function '%s', no overload defined</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode FUNC_OVERLOAD_NOT_FOUND = 9223373187906028632;
	
	/// <summary>
	/// <para>Code: HL1113</para>
	/// <para>Message: cannot overload operator '%s', no overload defined for types '%s' and '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode OP_OVERLOAD_NOT_FOUND_BINARY = 9223373187906028633;
	
	/// <summary>
	/// <para>Code: HL1114</para>
	/// <para>Message: cannot overload operator '%s', no overload defined for type '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode OP_OVERLOAD_NOT_FOUND_UNARY = 9223373187906028634;
	
	/// <summary>
	/// <para>Code: HL1115</para>
	/// <para>Message: '%s' is of type '%s', but a namespace was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_NAMESPACE_SYMBOL = 9223373187906028635;
	
	/// <summary>
	/// <para>Code: HL1116</para>
	/// <para>Message: '%s' is of type '%s', but a function was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_FUNC_SYMBOL = 9223373187906028636;
	
	/// <summary>
	/// <para>Code: HL1117</para>
	/// <para>Message: '%s' is of type '%s', but a operator was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_OP_SYMBOL = 9223373187906028637;
	
	/// <summary>
	/// <para>Code: HL1118</para>
	/// <para>Message: '%s' is of type '%s', but a constructor was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_CTOR_SYMBOL = 9223373187906028638;
	
	/// <summary>
	/// <para>Code: HL1119</para>
	/// <para>Message: '%s' is of type '%s', but a constructor or function was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_CTOR_OR_FUNC_SYMBOL = 9223373187906028639;
	
	/// <summary>
	/// <para>Code: HL1120</para>
	/// <para>Message: '%s' is of type '%s', but a struct was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_STRUCT_SYMBOL = 9223373187906028640;
	
	/// <summary>
	/// <para>Code: HL1121</para>
	/// <para>Message: '%s' is of type '%s', but a enum was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_ENUM_SYMBOL = 9223373187906028641;
	
	/// <summary>
	/// <para>Code: HL1122</para>
	/// <para>Message: '%s' is of type '%s', but a member or variable was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_IDENTIFIER_SYMBOL = 9223373187906028642;
	
	/// <summary>
	/// <para>Code: HL1123</para>
	/// <para>Message: '%s' is of type '%s', but a attribute was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_ATTRIBUTE_SYMBOL = 9223373187906028643;
	
	/// <summary>
	/// <para>Code: HL1124</para>
	/// <para>Message: '%s' is of type '%s', but a member was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_MEMBER_SYMBOL = 9223373187906028644;
	
	/// <summary>
	/// <para>Code: HL1125</para>
	/// <para>Message: '%s' is of type '%s', but a struct, class or enum type was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_TYPE_SYMBOL = 9223373187906028645;
	
	/// <summary>
	/// <para>Code: HL1126</para>
	/// <para>Message: '%s' is of type '%s', but a array type was expected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode EXPECTED_ARRAY_SYMBOL = 9223373187906028646;
	
	/// <summary>
	/// <para>Code: HL1127</para>
	/// <para>Message: '%s' is not a valid array element type</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode INVALID_ARRAY_TYPE = 9223373187906028647;
	
	/// <summary>
	/// <para>Code: HL1128</para>
	/// <para>Message: use of variable '%s' before its declaration</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode USE_BEFORE_DECL = 9223373187906028648;
	
	/// <summary>
	/// <para>Code: HL1129</para>
	/// <para>Message: couldn't resolve type of member '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode CANNOT_RESOLVE_MEMBER_TYPE = 9223373187906028649;
	
	/// <summary>
	/// <para>Code: HL1130</para>
	/// <para>Message: ambiguous function overload for '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode AMBIGUOUS_FUNC_OVERLOAD = 9223373187906028650;
	
	/// <summary>
	/// <para>Code: HL1131</para>
	/// <para>Message: ambiguous operator overload for '%s'</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode AMBIGUOUS_OP_OVERLOAD = 9223373187906028651;
	
	/// <summary>
	/// <para>Code: HL1200</para>
	/// <para>Message: recursive struct layout detected</para>
	/// <para>Description: Desc</para>
	/// <para>Category: Semantic Error</para>
	/// <para>Severity: Error</para>
	/// </summary>
	constexpr DiagnosticCode RECURSIVE_STRUCT_LAYOUT = 9223373187906028720;
	
}
#endif // LOCALIZATION_HPP
