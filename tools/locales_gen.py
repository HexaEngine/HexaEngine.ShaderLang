from genericpath import isdir
import zstandard as zstd
import os
import struct
import argparse
import ctypes

class RangeLookup:
    def __init__(self):
        self.range_map = {}

    def add_range(self, range_begin, range_end, text):
        range_begin_converted = encode_code_id("", range_begin)
        range_end_converted = encode_code_id("", range_end)

        for value in range(range_begin_converted, range_end_converted + 1):
            self.range_map[value] = text

    def value_in_range(self, value):
        value_converted = encode_code_id("", value)

        return self.range_map.get(value_converted, None)

def strip_markdown_formatting(text: str) -> str:
    result = []
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
    return ''.join(result)

def parse_range_label(lookup: RangeLookup, input_string):
    label, text = input_string.split(', ')
    _, range_part = label.split(': ')
    range_begin, range_end = range_part.split('-')
    
    lookup.add_range(strip_markdown_formatting(range_begin), strip_markdown_formatting(range_end), text);

def parse_markdown_file(file_path):
    messages = []
    header = None
    lookupCategory = RangeLookup()
    lookupSeverity = RangeLookup()


    with open(file_path, 'r') as f:
        for line in f:
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

                msg = {}
                for idx, column in enumerate(columns):
                    label = header[idx]

                    if label == 'code':
                        msg['code'] = strip_markdown_formatting(column)
                    elif label == 'name':
                        msg['name'] = column
                    elif label == 'message':
                        msg['message'] = column
                    elif label == 'description':
                        msg['description'] = column
                    elif label == 'category':
                        msg['category'] = column
                    elif label == 'severity':
                        msg['severity'] = column

                if not 'category' in msg:
                    msg['category'] = lookupCategory.value_in_range(msg['code']);

                if not 'severity' in msg:
                    msg['severity'] = lookupSeverity.value_in_range(msg['code']);

                messages.append(msg)
            else:
                header = None

    return messages

import math

base10Tobase2Digit = 3.321928094887362;

def encode_with_leading(input_str: str, start: int, length: int) -> int:
    number = 0

    for i in range(start, length):
        c = input_str[i]
        if c.isdigit():
            number = number * 10 + int(c)
        else:
            raise ValueError("Unexpected character in analyzer code id.")

    width = length - start;
    starting_bit = min(math.ceil(width * base10Tobase2Digit), 31)
    return number | (1 << starting_bit)

def encode_code_id(severity: str, input_str: str) -> int:
    severity = severity.lower()
    sevValue = 0
    if severity == "info":
        sevValue = 0
    if severity == "warning":
        sevValue = 1
    if severity == "error":
        sevValue = 2
    if severity == "critical":
        sevValue = 3
    prefix = 0
    i = 0
    length = min(len(input_str), 6)

    while i < length:
        c = input_str[i]
        if c.isalpha():
            prefix = (prefix << 5) | (ord(c.upper()) - ord('A') + 1)
            i += 1
        else:
            break

    prefix |= sevValue << 30
    combined_value = (prefix << 32) | encode_with_leading(input_str, i, length)
    return ctypes.c_uint64(combined_value & 0xFFFFFFFFFFFFFFFF).value

class CodeWriter:
    def __init__(self, f):
        self.f = f
        self.indentLevel = 0
        self.indentStr = "\t"
        self.indentCache = ""

    def indent(self):
        self.indentLevel += 1
        self.indentCache = self.indentStr * self.indentLevel

    def unindent(self):
        self.indentLevel = max(0, self.indentLevel - 1)
        self.indentCache = self.indentStr * self.indentLevel

    def write(self, text):
        self.f.write(self.indentCache + text)

    def writeln(self, text = ""):
        self.write(text + "\n")

    def beginblock(self, text):
        self.writeln(text)
        self.writeln('{')
        self.indent()

    def endblock(self, text = '}'):
        self.unindent()
        self.writeln(text)

def generate_header_file(output_file, messages):
    with open(output_file, 'w') as f:
        writer = CodeWriter(f)
        writer.writeln("#ifndef LOCALIZATION_HPP")
        writer.writeln("#define LOCALIZATION_HPP")
        writer.writeln()

        writer.writeln("#include <memory>")
        writer.writeln("#include <unordered_map>")
        writer.writeln("#include <string>")
        writer.writeln("#include \"diagnostic_code.hpp\"")
        writer.writeln("#include \"io/logger.hpp\"")
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
            writer.writeln(f"/// <para>Code: {msg['code']}</para>")
            writer.writeln(f"/// <para>Message: {msg['message']}</para>")
            writer.writeln(f"/// <para>Description: {msg['description']}</para>")
            writer.writeln(f"/// <para>Category: {msg['category']}</para>")
            writer.writeln(f"/// <para>Severity: {msg['severity']}</para>")
            writer.writeln("/// </summary>")
            writer.writeln(f"constexpr DiagnosticCode {msg['name']} = {int(encode_code_id(msg['severity'], msg['code']))};")
            writer.writeln()

        writer.endblock()
     
        writer.writeln("#endif // LOCALIZATION_HPP")

MAGIC_STRING = "TRANSL" 
CURRENT_VERSION = 1  

def write_translations(filename, messages):
    with open(filename, 'wb') as f:
        f.write(MAGIC_STRING.encode('utf-8'))
        f.write(struct.pack('I', CURRENT_VERSION))
        f.write(struct.pack('Q', len(messages)))

        cctx = zstd.ZstdCompressor(level=3)
        with cctx.stream_writer(f) as writer:
            for msg in messages:
                value_bytes = msg['message'].encode('utf-8') 
                str_length = struct.pack('I', len(value_bytes)) 
        
                writer.write(struct.pack('Q', encode_code_id("", msg['code'])))
                writer.write(str_length)
                writer.write(value_bytes)
            writer.flush(zstd.FLUSH_FRAME)

def extract_language_code(filename):
    first_dot_index = filename.find('.')
    if first_dot_index != -1:
        lang_code = filename[:first_dot_index]
        return lang_code
    return None

def main():
    parser = argparse.ArgumentParser(description="Generate .hpp files for localization")
    parser.add_argument('--locale', required=True, help="Locale code (e.g., en_US, de_DE, all)")
    parser.add_argument('--out_hpp', required=False, help="Output header path")
    parser.add_argument('--out_locales', required=False, help="Output locale transl file path")
    args = parser.parse_args()

    input_dir = f'resources/diagnostic_messages/'
    

    if args.locale == "all":
        os.makedirs(args.out_locales, exist_ok=True)
        messages = []
        for d in os.listdir(input_dir):
            dir_path = os.path.join(input_dir, d)
            if not d.startswith('.') and os.path.isdir(dir_path):
                messages_locale = []
                lang_code = d
                for root, _, files in os.walk(dir_path):
                    for filename in files:
                        file_path = os.path.join(root, filename)
                        msgs = parse_markdown_file(file_path);
                        messages_locale.extend(msgs)
                        messages.extend(msgs)

                write_translations(os.path.join(args.out_locales, f"{lang_code}.transl"), messages_locale)
    else:
        messages = []
        dir_path = os.path.join(input_dir, args.locale)
        for root, _, files in os.walk(dir_path):
            for filename in files:
                file_path = os.path.join(root, filename)
                messages.extend( parse_markdown_file(file_path))      
        os.makedirs(os.path.dirname(args.out_hpp), exist_ok=True)
        generate_header_file(args.out_hpp, messages)

if __name__ == '__main__':
    main()
