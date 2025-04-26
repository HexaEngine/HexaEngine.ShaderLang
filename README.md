# HXSLCompiler - HexaEngine Shading Language Compiler

## Overview

HXSL (HexaEngine Shading Language) is a modern shading language designed as a more flexible, readable, and user-friendly alternative to HLSL (High-Level Shading Language). Built with features like user-defined operators, member functions, and namespace support, HXSL enhances the shader programming experience by providing additional syntactical sugar. The goal is to improve developer productivity and code maintainability while keeping performance and compatibility with graphics hardware.

Currently, HXSL is in the **Semantic Analyzer** stage of development and is actively being worked on.

## Key Features

- **HLSL-inspired syntax**: HXSL takes inspiration from HLSL, making it familiar to developers with experience in DirectX shader programming.
- **User-defined Operators**: Allows for the creation of custom operators for greater expressiveness and readability in shader code.
- **Member Functions**: Supports defining functions within structures and types, improving modularity and code reuse.
- **Namespaces and Usings**: Removes the need for `#include` directives, allowing for better organization and cleaner code.
- **Ongoing Development**: Currently in the **Semantic Analyzer** phase, with features continually being added and improved.

## Features in Progress

- **Semantic Analyzer**: The compiler is currently focusing on parsing and analyzing code semantics, ensuring proper type checking and variable usage.
- **Type Checking**: Advanced type-checking is being developed to prevent errors and improve shader program correctness.

## Future Plans

- **Code Generation**: The next milestone will be to implement the code generation phase, producing optimized machine code or an intermediate representation for execution on the GPU.
- **Cross-compilation**: HXSL will eventually be compiled back to HLSL, and there are plans to extend support for generating GLSL and SPIR-V as well, enabling wider compatibility across different platforms and graphics APIs.
- **Extended Language Features**: More language features and optimizations will be added as development progresses.
- **C-Headers**: C-Headers for interop with other languages like C#.

### Prerequisites

- HXSL has no external dependencies and works with the C++17 standard library.

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
