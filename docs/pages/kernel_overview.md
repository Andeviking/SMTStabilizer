\page page_kernel_overview Kernel Overview

The kernel is the canonicalization core of SMTStabilizer.

## Responsibilities

- Build graph and reverse graph views over DAG nodes.
- Propagate context-sensitive hashes across expression structure.
- Resolve symmetry and tie cases with selective perturbation.
- Rebuild active graph slices when pruning indistinguishable regions.
- Normalize symbol/function naming order for deterministic output.

## Main Entry Point

- `stabilizer::kernel::Kernel::apply`

## Core Internal Stages

- `context_propagate`: fixed-point propagation over a processing slice.
- `specific_propagate`: localized perturbation for collisions.
- `rebuild_graph`: keep only currently ambiguous subgraph nodes.
- `sort_propagate`: include sort/datatype influence in ordering.
