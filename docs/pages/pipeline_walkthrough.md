\page page_pipeline_walkthrough Pipeline Walkthrough

This walkthrough tracks one stabilization call from API entry to final serialized output.

## Step 1: API Input Handling

- API rejects empty file/text input.
- Parser is created and configured with keep-let disabled, function expansion disabled, and the rewrite toggle forwarded.
- If check-sat is enabled, the stabilized assertions are forwarded to the selected solver backend and a solver status string is returned.

## Step 2: Node Manager Preparation

- `NodeManager` wraps parser state.
- Initial assertions are simplified before kernel processing.

## Step 3: Kernel Fixed Point and Refinement

- Initial context propagation computes structural hashes.
- Optional sort propagation incorporates datatype/sort discrimination.
- Specific propagation resolves remaining hash collisions.
- Graph rebuild limits work to unresolved regions.

## Step 4: Canonical Naming and Output

- Symbols, UFs, datatype constructors/selectors, and function declarations are renamed deterministically.
- Assertions, function declarations, and datatype blocks are sorted by stabilized ordering keys.
- Final SMT2 text is emitted through node manager serialization unless check-sat mode is enabled, in which case the solver status string is returned.

## Source Anchors

- `Kernel::apply`
- `Kernel::context_propagate`
- `Kernel::specific_propagate`
- `Kernel::rebuild_graph`
- `Kernel::sort_propagate`
