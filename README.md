# SMTStabilizer

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

SMTStabilizer improves the stability of SMT solvers by applying a
context-driven approximate normalization to input formulas. The tool reduces runtime
variance caused by small syntactic mutations (e.g. assertion reordering, symbol renaming,
commutative operand reordering) and produces a nearly canonical form that is consistent
across inputs.

## Overview

Satisfiability Modulo Theories (SMT) solvers are widely used in formal verification.In program analysis, users often encounter queries that differ only by simple syntactic mutations and are logically equivalent. These mutations typically include assertion reordering, symbol renaming, antisymmetric relation inversion, and commutative operand reordering. 

However, such minor changes can cause runtimes to vary by orders of magnitude. This variability reduces the predictability required for industrial-scale verification and remains a critical challenge. 

SMTStabilizer, a tool that improves the stability of SMT solvers via context-driven normalization. Since complete input normalization has been shown to be as hard as the graph isomorphism problem, SMTStabilizer adopts an approximate normalization strategy to avoid the high cost of exact normalization. 

The framework converts formulas into a structured representation and propagates structural information across nodes, so that each node becomes aware of its surrounding context. Using these context information, SMTStabilizer derives a consistent ordering over subformulas. This process yields a nearly canonical form that remains consistent across isomorphic inputs. 

SMTStabilizer also leverages pruning techniques utilizing the syntactic structure of SMT to reduce the normalization time. An evaluation with Z3 and cvc5 on millions of queries shows that SMTStabilizer improves the solvers’ stability to over 98% under 10 random mutations.

## Table of Contents

- Quick Start
- Build & Install
- Contributing
- Security
- License
- Acknowledgements

## Quick Start

Minimal steps to build and run on a Unix-like system.

1. Clone the repository

```bash
git clone https://github.com/Andeviking/SMTStabilizer.git
cd SMTStabilizer
```

2. Build submodules (this script will download and build bundled GMP/MPFR)

```bash
python setup.py
```

3. Configure and build the project

```bash
cmake -S . -B build
cmake --build build -j
```

4. Run the binary

```bash
./build/bin/SMTStabilizer --help
# example: ./build/bin/SMTStabilizer input.smt2 > output.smt2
```

## Build & Install

Prerequisites (examples; replace placeholders as needed):

- A C++20-capable compiler (GCC 10+/Clang 10+ recommended).
- CMake (>= 3.15 recommended).
- Python 3.8+ (used by `setup.py`).


## Contributing

Contributions are welcome. Please open issues for bugs or feature requests and
submit pull requests with clear descriptions and tests where applicable. See
CONTRIBUTING.md for contribution guidelines (if present). If you would like
me to add a CONTRIBUTING.md, I can scaffold one for you.

## Security

If you discover a security vulnerability, please contact the maintainers at
gandeviking@gmail.com or open a private issue.

## License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.

## Acknowledgements

- The parser implementation in `src/parser/` is based on the open-source SOMTParser project (https://github.com/fuqi-jia/SOMTParser). Modifications and extensions have been made by us to adapt the parser for SMTStabilizer's normalization framework and additional requirements.