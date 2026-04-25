\page page_api_kernel_boundary API and Kernel Boundary

This page clarifies where public API responsibilities end and internal kernel responsibilities begin.

## Boundary Contract

- API layer (`src/api`) validates input text/file paths, configures parser options, and orchestrates execution.
- Kernel layer (`src/kernel`) computes canonical ordering and symbol renaming to stabilize output.
- Node manager (`src/node`) is the bridge used by both layers to access and update DAG assertions.

## Execution Path

`SMTStabilizer::apply_file` or `SMTStabilizer::apply_text`
-> `SMTStabilizer::run_pipeline`
-> `NodeManager::simplify_assertions`
-> `Kernel::apply`
-> `NodeManager::to_string`

## Why This Separation Exists

- API remains stable for external integrations.
- Kernel can evolve internal heuristics without breaking API signatures.
- Debugging is easier because the transition point is explicit at `run_pipeline`.
