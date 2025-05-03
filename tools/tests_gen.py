import os

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

    def write(self, text: str):
        self.f.write(self.indentCache + text)

    def writeln(self, text = ""):
        self.write(text + "\n")

    def beginblock(self, text: str, delimiter = '{'):
        self.writeln(text)
        self.writeln(delimiter)
        self.indent()

    def endblock(self, text = '}'):
        self.unindent()
        self.writeln(text)

def extract_test_name:

def generate_test_suite(test_directory: str, output_file: str, includes: list[str], base_class: str, test_group: str) -> None:
    with open(output_file, "w") as f:
        writer = CodeWriter(f)

        if includes:
            for include in includes:
                writer.writeln(f'#include "{include}"')

        writer.beginblock('TEST_P(PrattParserTest, TestWithParameter)')
        writer.writeln('Act();')
        writer.endblock()

        writer.beginblock('INSTANTIATE_TEST_SUITE_P', '(')
        writer.writeln(f'{test_group},')
        writer.writeln(f'{base_class},')
        writer.beginblock('::testing::Values', '(')

        test_files = [f for f in os.listdir(test_directory) if f.endswith(".txt")]
        
        for test_file in test_files:
            test_name = test_file.replace(".txt", "")
            writer.writeln(f'std::make_tuple("{test_directory}{test_file}", "{test_name}"),')

        writer.endblock('),')

        writer.beginblock("[](const testing::TestParamInfo<PrattParserTest::ParamType>& info)")
        writer.writeln('return std::get<1>(info.param);')
        writer.endblock()

        writer.endblock(');')

def main():
    generate_test_suite("", "")

if __name__ == '__main__':
    main()
