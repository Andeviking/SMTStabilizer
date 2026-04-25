\page page_faq FAQ

## Why does the API expose only high-level options?

The API is intended to remain stable for integrations. Internal kernel strategy can evolve independently.

## Why are private kernel methods documented?

Maintainers need source-linked stage descriptions to debug behavior regressions and ordering changes.

## Does this documentation include full parser internals?

Not yet. Current scope is API plus kernel, with parser details only when needed for handoff explanation.

## How should warnings be used?

Warnings are enabled to highlight missing docs and param details, but they can be gated in CI gradually.
