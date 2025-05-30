import os
import argparse
from typing import Optional
from ctypes import c_uint64
from code_writer import CodeWriter
from diag_message import DiagMessage, Severity, encode_code_id
from transl_writer import write_translations


class RangeLookup:
    range_map: dict[int, str]

    def __init__(self):
        self.range_map = dict[int, str]()

    def add_range(self, range_begin: str, range_end: str, text: str):
        range_begin_converted = encode_code_id(Severity.Info, range_begin)
        range_end_converted = encode_code_id(Severity.Info, range_end)

        for value in range(range_begin_converted.value, range_end_converted.value + 1):
            self.range_map[value] = text

    def value_in_range(self, value: int) -> Optional[str]:
        return self.range_map.get(value, None)
    
    def get_value(self, value: c_uint64) -> Optional[str]:
        return self.range_map.get(value.value, None)

def strip_markdown_formatting(text: str) -> str:
    result = list[str]()
    skip_chars = {'`', '*', '_', '~'}
    i = 0
    while i < len(text):
        c = text[i]
        if c == '\\' and i + 1 < len(text):
            result.append(text[i + 1])
            i += 2
            continue
        if c in skip_chars:
            i += 1
            continue
        result.append(c)
        i += 1
    return ''.join(result).strip()

def parse_range_label(lookup: RangeLookup, input_string: str):
    label, text = input_string.split(', ')
    _, range_part = label.split(': ')
    range_begin, range_end = range_part.split('-')
    
    lookup.add_range(strip_markdown_formatting(range_begin), strip_markdown_formatting(range_end), text)

def parse_markdown_file(file_path: str, seen_codes: set[int]) -> list[DiagMessage]:
    messages = list[DiagMessage]()
    header = None
    lookupCategory = RangeLookup()
    lookupSeverity = RangeLookup()

    line_index = 0
    with open(file_path, 'r') as f:
        for line in f:
            line_index += 1
            line = line.strip()

            if line.startswith('-'):
                part = line.lstrip('- ')
                index = part.find(':')
                if index == -1:
                    continue

                part = strip_markdown_formatting(part[:index]).lower()
                if part == "category":
                    parse_range_label(lookupCategory, line)
                if part == "severity":
                    parse_range_label(lookupSeverity, line)

            if "|" in line:
                if not header:
                    header = [col.strip().lower() for col in line.split("|")[1:-1]]
                    continue
              
                columns = [col.strip() for col in line.split("|")[1:-1]]

                if not columns or columns[0].strip('-') == '':
                    continue
                
                if len(columns) != len(header):
                    continue

                msg = DiagMessage()
                for idx, column in enumerate(columns):
                    label = header[idx]

                    if label == 'code':
                        msg.code = strip_markdown_formatting(column)
                    elif label == 'name':
                        msg.name = column
                    elif label == 'message':
                        msg.message = column
                    elif label == 'description':
                        msg.description = column
                    elif label == 'category':
                        msg.category = column
                    elif label == 'severity':
                        msg.severity = Severity[column.capitalize()]

                msg.populate_lookup_id()

                if not msg.category:
                    msg.category = lookupCategory.get_value(msg.lookup_id)

                if not msg.severity:
                    result = lookupSeverity.get_value(msg.lookup_id)
                    if result:
                        msg.severity = Severity[result.capitalize()]
                        
                msg.populate_code_id()

                if not msg.validate(file_path, line_index):
                    continue

                if msg.lookup_id.value in seen_codes:
                    print(f"[Warning]: Duplicate code id {msg.code} in diagnostic message (Line: {line_index}) {file_path}")
                    continue

                seen_codes.add(msg.lookup_id.value)

                messages.append(msg)
            else:
                header = None

    return messages


def generate_header_file(output_file: str, messages: list[DiagMessage]):
    with open(output_file, 'w') as f:
        writer = CodeWriter(f)
        writer.writeln("#ifndef LOCALIZATION_HPP")
        writer.writeln("#define LOCALIZATION_HPP")
        writer.writeln()

        writer.writeln("#include <memory>")
        writer.writeln("#include <unordered_map>")
        writer.writeln("#include <string>")
        writer.writeln("#include \"logging/diagnostic_code.hpp\"")
        writer.writeln("#include \"logging/logger.hpp\"")
        writer.writeln()

        writer.writeln("// Automatically generated locale codes")
        writer.writeln()

        writer.beginblock("namespace HXSL")

        writer.writeln(f"extern std::unique_ptr<std::unordered_map<uint64_t, std::string>> current_locale_map;")
        writer.writeln()

        writer.writeln("extern void SetLocale(const std::string& language_code);")
        writer.writeln("extern std::string GetMessageForCode(uint64_t code);")
        writer.writeln("extern std::string GetStringForCode(uint64_t code);")
        writer.writeln("extern LogLevel GetLogLevelForCode(uint64_t code);")
        writer.writeln("extern uint64_t EncodeCodeId(LogLevel level, const std::string& input);")
        writer.writeln()

        for msg in messages:
            writer.writeln("/// <summary>")
            writer.writeln(f"/// <para>Code: {msg.code}</para>")
            writer.writeln(f"/// <para>Message: {msg.message}</para>")
            writer.writeln(f"/// <para>Description: {msg.description}</para>")
            writer.writeln(f"/// <para>Category: {msg.category}</para>")
            writer.writeln(f"/// <para>Severity: {(msg.severity or Severity.Info).name}</para>")
            writer.writeln("/// </summary>")
            writer.writeln(f"constexpr DiagnosticCode {msg.name} = {int(msg.code_id.value)};")
            writer.writeln()

        writer.endblock()
     
        writer.writeln("#endif // LOCALIZATION_HPP")

def extract_language_code(filename: str) -> Optional[str]:
    first_dot_index = filename.find('.')
    if first_dot_index != -1:
        lang_code = filename[:first_dot_index]
        return lang_code
    return None

def parse_locale_messages(messages: list[DiagMessage], messages_locale: list[DiagMessage], dir_path: str):
    seen_codes = set[int]()
    for root, _, files in os.walk(dir_path):
        for filename in files:
            file_path = os.path.join(root, filename)
            msgs = parse_markdown_file(file_path, seen_codes)
            messages_locale.extend(msgs)
            messages.extend(msgs)

def main():
    parser = argparse.ArgumentParser(description="Generate .hpp files for localization")
    parser.add_argument('--locale', required=True, help="Locale code (e.g., en_US, de_DE, all)")
    parser.add_argument('--out_hpp', required=False, help="Output header path")
    parser.add_argument('--out_locales', required=False, help="Output locale transl file path")
    args = parser.parse_args()

    input_dir = f'resources/diagnostic_messages/'
    
    messages = list[DiagMessage]()

    if args.locale == "all":
        os.makedirs(args.out_locales, exist_ok=True)
        for dir_name in os.listdir(input_dir):
            dir_path = os.path.join(input_dir, dir_name)
            if not dir_name.startswith('.') and os.path.isdir(dir_path):
                messages_locale = list[DiagMessage]()
                parse_locale_messages(messages, messages_locale, dir_path)
                write_translations(os.path.join(args.out_locales, f"{dir_name}.transl"), messages_locale)
    else:
        os.makedirs(os.path.dirname(args.out_hpp), exist_ok=True)
        dir_path = os.path.join(input_dir, args.locale)
        parse_locale_messages(messages, [], dir_path)
        generate_header_file(args.out_hpp, messages)

if __name__ == '__main__':
    main()
