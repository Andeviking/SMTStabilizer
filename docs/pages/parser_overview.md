\page page_parser_overview Parser Overview

The parser module (`src/parser`) is responsible for ingesting SMT-LIB2 input and producing DAG-based internal nodes used by later stabilization stages.

## Responsibilities

- Parse SMT-LIB2 commands and expressions from file/text.
- Build typed DAG nodes and maintain symbol/sort/function tables.
- Track assertion sets, assumptions, and optional metadata.
- Offer rewrite hooks used during normalization-friendly construction.

## Main Entry Points

- `stabilizer::parser::Parser::parse`
- `stabilizer::parser::Parser::parseStr`
- `stabilizer::parser::Parser::mkExpr`

## Key Supporting Types

- `stabilizer::parser::DAGNode` for expression graph nodes.
- `stabilizer::parser::Sort` and `stabilizer::parser::SortManager` for sort handling.
- `stabilizer::parser::Value` and numeric wrappers for constant interpretation.

## Boundary Notes

- Parser is the producer of node-level semantic structure.
- Kernel consumes this structure through node manager views; parser should not assume kernel-specific ordering or naming conventions.
