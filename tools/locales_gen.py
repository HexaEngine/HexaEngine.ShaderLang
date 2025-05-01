import zstandard as zstd
import os
import struct
import argparse

class RangeLookup:
    def __init__(self):
        self.range_map = {}

    def add_range(self, range_begin, range_end, text):
        range_begin_converted = encode_code_id(range_begin)
        range_end_converted = encode_code_id(range_end)

        for value in range(range_begin_converted, range_end_converted + 1):
            self.range_map[value] = text

    def value_in_range(self, value):
        value_converted = encode_code_id(value)

        return self.range_map.get(value_converted, None)

def parse_range_label(lookup: RangeLookup, input_string):
    label, text = input_string.split(', ')
    _, range_part = label.split(': ')
    range_begin, range_end = range_part.split('-')
    
    lookup.add_range(range_begin, range_end, text);

def parse_markdown_file(file_path):
    messages = []
    header = None
    lookupCategory = RangeLookup()
    lookupSeverity = RangeLookup()


    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()

            if line.startswith("#") or not line:
                continue

            if line.startswith('- Category:'):
                parse_range_label(lookupCategory, line)

            if line.startswith('- Severity:'):
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
                        msg['code'] = column
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

def encode_with_leading(input_str: str, start: int, length: int) -> int:
    number = 0

    for i in range(start, length):
        c = input_str[i]
        if c.isdigit():
            number = number * 10 + int(c)
        else:
            raise ValueError("Unexpected character in analyzer code id.")

    limit = 31
    starting_bit = min(math.ceil(math.log2(length - start)), limit)

    return number | (1 << starting_bit)

def encode_code_id(input_str: str) -> int:
    prefix = 0
    i = 0
    length = len(input_str)

    while i < length:
        c = input_str[i]
        if c.isalpha():
            prefix = (prefix << 5) | (ord(c.upper()) - ord('A') + 1)
            i += 1
        else:
            break

    return (prefix << 32) | encode_with_leading(input_str, i, length)

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
        writer.writeln()

        writer.writeln("// Automatically generated locale codes")
        writer.writeln()

        writer.beginblock("namespace HXSL::Codes")

        writer.writeln(f"extern std::unique_ptr<std::unordered_map<uint64_t, std::string>> current_locale_map;")
        writer.writeln()

        writer.writeln("static void SetLocale(const std::string& language_code);")
        writer.writeln("static std::string GetMessageForCode(uint64_t code);")
        writer.writeln("static std::string GetStringForCode(uint64_t code);")
        writer.writeln()

        for msg in messages:
            writer.writeln(f"// Code: {msg['code']}")
            writer.writeln(f"// Message: {msg['message']}")
            writer.writeln(f"// Description: {msg['description']}")
            writer.writeln(f"// Category: {msg['category']}")
            writer.writeln(f"// Severity: {msg['severity']}")
            writer.writeln(f"constexpr uint64_t {msg['code']} = {bin(encode_code_id(msg['code']))};")
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
        
                writer.write(struct.pack('Q', encode_code_id(msg['code'])))
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
    parser.add_argument('--out_hpp', required=True, help="Output path")
    parser.add_argument('--out_locales', required=True, help="Output path")
    args = parser.parse_args()

    input_dir = f'resources/localizable_errors/'
    os.makedirs(os.path.dirname(args.out_hpp), exist_ok=True)
    os.makedirs(args.out_locales, exist_ok=True)

    if args.locale == "all":
        messages = []
        for file in os.listdir(input_dir):
            full_path = os.path.join(input_dir, file)
            if os.path.isfile(full_path):
                msgs = parse_markdown_file(full_path);
                messages.extend(msgs)
                write_translations(os.path.join(args.out_locales, f"{extract_language_code(file)}.transl"), msgs)

             
        generate_header_file(args.out_hpp, messages)
    else:
        input_file = f'{input_dir}/{args.locale}.hxsl.syntax.md'
        messages = parse_markdown_file(input_file)        
        generate_header_file(args.out_hpp, messages)

if __name__ == '__main__':
    main()
