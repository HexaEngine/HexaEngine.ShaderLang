#ifndef LOCALIZATION_HPP
#define LOCALIZATION_HPP

#include <memory>
#include <unordered_map>
#include <string>

// Automatically generated locale codes

namespace HXSL::Codes
{
	extern std::unique_ptr<std::unordered_map<uint64_t, std::string>> current_locale_map;
	
	static void SetLocale(const std::string& language_code);
	static std::string GetMessageForCode(uint64_t code);
	static std::string GetStringForCode(uint64_t code);
	
	// Code: HL001
	// Message: invalid token
	// Description: A token that is not recognized by the lexer. This often occurs due to a typo or unsupported character.
	// Category: Syntax Error
	// Severity: Error
	constexpr uint64_t HL001 = 0b10000110000000000000000000000000000000101;
	
	// Code: HL002
	// Message: expected ';'
	// Description: A semicolon (;) was expected at the end of a statement but was missing.
	// Category: None
	// Severity: None
	constexpr uint64_t HL002 = 0b10000110000000000000000000000000000000110;
	
}
#endif // LOCALIZATION_HPP
