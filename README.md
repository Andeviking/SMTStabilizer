# SMTStabilizer

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Docs](https://img.shields.io/badge/docs-Doxygen-informational.svg)](docs/)

SMTStabilizer is a normalization tool for SMT-LIB2 inputs. It reduces runtime
variance caused by small syntactic mutations such as assertion reordering,
symbol renaming, and commutative operand reordering, then emits a more stable
SMT2 form for downstream solvers.

## Overview

SMT solvers are widely used in formal verification and program analysis. In practice, small changes in inputs can still trigger large runtime variance, which makes solver behavior harder to predict. Unfortunately, it has been shown that normalizing such mutations is equivalent to solving the graph isomorphism problem, which is infeasible on industrial-scale graphs.

SMTStabilizer addresses that problem with an approximate normalization algorithm.
It parses SMT-LIB2, builds the internal node representation, propagates context
information, applies kernel-based pruning, and finally emits normalized SMT2
text.

## Features

- CLI tool for normalizing SMT-LIB2 files.
- Public C++ API for embedding the normalization pipeline in other projects.
- Public C API for use from C or foreign-language bindings.
- Configurable rewrite, context propagation, and subgraph pruning flags.
- Doxygen documentation generated from the public API headers.
- CTest-based API tests.

## Project Layout

- `src/main/` command-line entry point.
- `src/api/` public C++ and C APIs.
- `src/parser/` SMT-LIB2 parser and rewrite logic.
- `src/kernel/` normalization kernel.
- `src/node/` node manager used by kernel.
- `test/` API tests and sample SMT2 fixtures.
- `docs/` generated API documentation.

## Build

Prerequisites:

- A C++20-capable compiler such as GCC 10+ or Clang 10+.
- CMake 3.15 or newer.
- Python 3.8+ for dependency setup.

Typical build steps:

```bash
python setup.py
cmake -S . -B build
cmake --build build -j
```

Run the CLI after building:

```bash
./build/bin/SMTStabilizer --help
./build/bin/SMTStabilizer input.smt2 > output.smt2
```

## API

The public API is exposed through the headers in `src/api/`.

### C++ API

Use `stabilizer::api::SMTStabilizerOptions` to configure the pipeline, then call
`stabilizer::api::SMTStabilizer::apply_file()` or
`stabilizer::api::SMTStabilizer::apply_text()`.

Minimal example:

```cpp
#include "api/stabilizer_api.h"

int main() {
	stabilizer::api::SMTStabilizerOptions options;
	options.set_rewrite(true);
	options.set_context_propagation(true);
	options.set_subgraph_pruning(true);

	stabilizer::api::SMTStabilizer stabilizer(options);
	std::string normalized = stabilizer.apply_file("input.smt2");
	return normalized.empty();
}
```

### C API

The C API uses opaque handles and returns heap-allocated result strings that
must be released with `stabilizer_free_string()`.

Minimal example:

```c
#include "api/stabilizer_c_api.h"

int main(void) {
	stabilizer_options *options = stabilizer_options_create();
	stabilizer_options_set_rewrite(options, true);
	stabilizer_options_set_context_propagation(options, true);
	stabilizer_options_set_subgraph_pruning(options, true);

	stabilizer_handle *handle = stabilizer_create(options);
	char *output = NULL;
	if (stabilizer_apply_file(handle, "input.smt2", &output) == STABILIZER_STATUS_OK) {
		/* use output */
		stabilizer_free_string(output);
	}

	stabilizer_destroy(handle);
	stabilizer_options_destroy(options);
	return 0;
}
```

## Documentation
### Installing Doxygen

#### Ubuntu/Debian
```bash
sudo apt-get install doxygen
```

#### CentOS/RHEL
```bash
sudo yum install doxygen
```

Generate the API documentation with the dedicated CMake target:

```bash
cmake --build build --target docs
```

The generated documentation is written to `docs/html/index.html`.

Open it in any browser, or from VS Code use one of these options:

```bash
xdg-open docs/html/index.html
```

or open the file directly from the editor sidebar.

## Testing

Run the API test suite with CTest after building:

```bash
ctest --test-dir build --output-on-failure
```

The tests live in `test/api_tests.cpp` and use the fixtures in `test/inputs/`.

## Contributing

Contributions are welcome. Please open issues for bugs or feature requests and
submit pull requests with clear descriptions and tests where applicable.

## Security

If you discover a security vulnerability, please contact the maintainers at
gandeviking@gmail.com or open a private issue.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file
for details.

## Acknowledgements

- The parser implementation in `src/parser/` is based on the open-source
  SOMTParser project (https://github.com/fuqi-jia/SOMTParser). We have substantially modified and extended it to fit SMTStabilizer’s normalization requirements.