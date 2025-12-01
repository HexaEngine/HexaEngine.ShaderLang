# HXSLCompiler - HexaEngine Shading Language Compiler

## Overview

HXSL (HexaEngine Shading Language) is a modern shading language designed as a more flexible, readable, and user-friendly alternative to HLSL (High-Level Shading Language). Built with features like user-defined operators, member functions, and namespace support, HXSL enhances the shader programming experience by providing additional syntactical sugar. The goal is to improve developer productivity and code maintainability while keeping performance and compatibility with graphics hardware.

Currently, HXSL is in the **Intermediate Language (IL) Optimization** stage of development. The frontend (lexer, parser, semantic analyzer) and the middle-end (IL generation, SSA conversion, and optimizations) are complete. Active work is now focused on backend code generation.

## Key Features

- **HLSL-inspired syntax**: HXSL takes inspiration from HLSL, making it familiar to developers with experience in DirectX shader programming.
- **User-defined Operators**: Allows for the creation of custom operators for greater expressiveness and readability in shader code.
- **Member Functions**: Supports defining functions within structures and types, improving modularity and code reuse.
- **Namespaces and Usings**: Removes the need for `#include` directives, allowing for better organization and cleaner code.
- **C API**: A C API is available for integration with other languages like C#.

## Compiler Architecture

The compiler is structured into the following stages:

### Completed Stages

- **Preprocessing**: Macro expansion and preprocessor directive handling.
- **Lexical Analysis**: Tokenization of HXSL source code.
- **Parsing**: Full parsing support for declarations, expressions, and statements using a hybrid Pratt parser.
- **Semantic Analysis**: Complete type checking, symbol resolution, and expression validation.
- **IL Generation**: Conversion of the AST to an intermediate representation (Module Builder).
- **Control Flow Analysis**: Construction and analysis of control flow graphs.
- **SSA Conversion**: Static Single Assignment form conversion for optimization.
- **IL Optimizations**:
  - Constant Folding
  - Algebraic Simplification
  - Common Subexpression Elimination
  - Dead Code Elimination
  - Strength Reduction
  - Function Inlining

### In Progress

- **Backend Code Generation**: Implementation of code generation targeting HLSL, GLSL, and SPIR-V.

## Future Plans

- **Cross-compilation**: HXSL will be transpiled to HLSL, with plans to extend support for generating GLSL and SPIR-V, enabling wider compatibility across different platforms and graphics APIs.
- **Extended Language Features**: More language features and optimizations will be added as development progresses.

### Prerequisites

- HXSL uses the C++20 standard library.

### Syntax and Usage

Refer to the documentation or your specific use cases for syntax examples. (Being worked on)

## Contributing

We welcome contributions! If you would like to help improve HXSL, feel free to open an issue or submit a pull request. Here's how you can contribute:

- **Fork the repository**.
- **Clone your fork** locally:
    ```bash
    git clone https://github.com/HexaEngine/HexaEngine.ShaderLang.git
    ```
- **Create a new branch** for your feature or bug fix.
- **Commit your changes** and push them to your fork.
- **Open a pull request** to propose your changes.

## License

This project is licensed under the MIT License. See the [LICENSE](https://github.com/HexaEngine/HexaEngine.ShaderLang/blob/master/LICENSE.txt) file for more details.
