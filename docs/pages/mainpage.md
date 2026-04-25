# SMTStabilizer Documentation Main Page

Welcome to the combined documentation for SMTStabilizer API, parser, kernel, and utility modules.

## Scope

This documentation intentionally separates two audiences:

- Public API users: start from API classes and C/C++ entry points under `src/api`.
- Maintainers: follow parser ingestion (`src/parser`), kernel normalization (`src/kernel`), and shared helpers (`src/util`).

Parser internals are documented at interface level. Utility helpers are documented around behavior contracts and intended call sites.

## Documentation Map

- \subpage page_api_kernel_boundary "API and Kernel Boundary"
- \subpage page_parser_overview "Parser Overview"
- \subpage page_kernel_overview "Kernel Overview"
- \subpage page_util_overview "Utility Overview"
- \subpage page_pipeline_walkthrough "Pipeline Walkthrough"
- \subpage page_glossary "Glossary"
- \subpage page_faq "FAQ"

## Quick Start

1. Read the API facade class `stabilizer::api::SMTStabilizer`.
2. Follow parser entry points in `stabilizer::parser::Parser` and related sort/value helpers.
3. Continue into `stabilizer::node::NodeManager` and `stabilizer::kernel::Kernel` for normalization.
4. Use utility references (`stabilizer::util`) when tracking hashing and bitvector/runtime helpers.
