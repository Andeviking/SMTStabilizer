\page page_util_overview Utility Overview

The utility module (`src/util`) provides reusable helpers used by parser/kernel and related infrastructure.

## Responsibilities

- Hash propagation helpers used by kernel ordering stages.
- Bitvector and GMP utility wrappers for cross-platform numeric behavior.
- Node-level helper functions shared by normalization logic.

## Scope Guidance

- Utility functions should remain deterministic and side-effect light.
- Most utility APIs are internal support contracts rather than public integration APIs.

## Main References

- `stabilizer::util::hash_node`
- `stabilizer::util::hash_children`
- `stabilizer::util::hash_parents`
- `stabilizer::util::BitVector`
