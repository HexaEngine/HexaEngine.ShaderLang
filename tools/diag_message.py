from typing import Optional
from enum import IntEnum
from ctypes import c_uint64

class Severity(IntEnum):
    Info = 0
    Warn = 1
    Error = 2
    Critical = 3

lut = [0, 4, 7, 10, 14, 17, 20, 24, 27, 30, 31]
def encode_with_leading(input_str: str, start: int, length: int) -> int:
    number = 0

    for i in range(start, length):
        c = input_str[i]
        if c.isdigit():
            number = number * 10 + int(c)
        else:
            raise ValueError("Unexpected character in diagnostic code.")

    range_value = length - start
    starting_bit = lut[range_value] if range_value < 11 else lut[10]
    return number | (1 << starting_bit)

def encode_code_id(severity: Severity, input_str: str) -> c_uint64:
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

    prefix |= severity.value << 30
    combined_value = (prefix << 32) | encode_with_leading(input_str, i, length)
    return c_uint64(combined_value & 0xFFFFFFFFFFFFFFFF)

class DiagMessage:
    code: Optional[str]
    code_id: c_uint64
    lookup_id: c_uint64
    name: Optional[str]
    message: Optional[str]
    description: Optional[str]
    category: Optional[str]
    severity: Optional[Severity]

    def __init__(self):
        self.code = None
        self.code_id = c_uint64(0)
        self.lookup_id = c_uint64(0)
        self.name = None
        self.message = None
        self.description = None
        self.category = None
        self.severity = None

    def populate_lookup_id(self):
        if self.code:
            self.lookup_id = encode_code_id(Severity.Info, self.code)
            
    def populate_code_id(self):
        if self.code:
            self.code_id = encode_code_id(self.severity or Severity.Info, self.code)

    def validate(self, filename: str, line: int) -> bool:
        missing_fields = list[str]()
        error = False
        
        if not self.code:
            missing_fields.append("code")
            error |= True
        if not self.name:
            missing_fields.append("name")
            error |= True
        if not self.message:
            missing_fields.append("message")
            error |= True
        if not self.description:
            missing_fields.append("description")
        if not self.category:
            error |= True
            missing_fields.append("category")
        if not self.severity:
            error |= True
            missing_fields.append("severity")

        if missing_fields:
            print(f"[{"Error" if error else "Warn"}]: Missing required fields: {', '.join(missing_fields)} in diagnostic message (Line: {line}) {filename}")
            return not error
        
        return True
