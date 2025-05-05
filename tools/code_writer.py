from typing import TextIO

class CodeWriter:
    def __init__(self, f: TextIO):
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

    def write(self, text: str):
        self.f.write(self.indentCache + text)

    def writeln(self, text: str = ""):
        self.write(text + "\n")

    def beginblock(self, text: str, mark: str = '{'):
        self.writeln(text)
        self.writeln(mark)
        self.indent()

    def endblock(self, text: str = '}'):
        self.unindent()
        self.writeln(text)