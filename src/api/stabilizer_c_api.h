// Copyright (c) 2026 XiangZhang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque public options object used by the C API. */
typedef struct stabilizer_options stabilizer_options;
/** Opaque stabilizer instance used by the C API. */
typedef struct stabilizer_handle stabilizer_handle;

typedef enum stabilizer_status {
    STABILIZER_STATUS_OK = 0,
    STABILIZER_STATUS_INVALID_ARGUMENT = 1,
    STABILIZER_STATUS_RUNTIME_ERROR = 2
} stabilizer_status;

/** Create and destroy public options. */
stabilizer_options *stabilizer_options_create(void);
void stabilizer_options_destroy(stabilizer_options *options);

/** Set or query the rewrite normalization flag. */
void stabilizer_options_set_rewrite(stabilizer_options *options, bool value);
bool stabilizer_options_get_rewrite(const stabilizer_options *options);

/** Set or query the kernel context propagation flag. */
void stabilizer_options_set_context_propagation(stabilizer_options *options, bool value);
bool stabilizer_options_get_context_propagation(const stabilizer_options *options);

/** Set or query the kernel subgraph pruning flag. */
void stabilizer_options_set_subgraph_pruning(stabilizer_options *options, bool value);
bool stabilizer_options_get_subgraph_pruning(const stabilizer_options *options);

/** Create and destroy a stabilizer instance. */
stabilizer_handle *stabilizer_create(const stabilizer_options *options);
void stabilizer_destroy(stabilizer_handle *handle);

/** Set or query the rewrite normalization flag on a live stabilizer. */
void stabilizer_set_rewrite(stabilizer_handle *handle, bool value);
bool stabilizer_get_rewrite(const stabilizer_handle *handle);

/** Set or query the kernel context propagation flag on a live stabilizer. */
void stabilizer_set_context_propagation(stabilizer_handle *handle, bool value);
bool stabilizer_get_context_propagation(const stabilizer_handle *handle);

/** Set or query the kernel subgraph pruning flag on a live stabilizer. */
void stabilizer_set_subgraph_pruning(stabilizer_handle *handle, bool value);
bool stabilizer_get_subgraph_pruning(const stabilizer_handle *handle);

/**
 * Apply the pipeline to a file or text input.
 *
 * On success, *output receives a heap-allocated UTF-8 string that must be
 * released with stabilizer_free_string.
 *
 * The handle remains valid after the call. Callers should check the returned
 * status and inspect stabilizer_last_error() when a non-OK status is returned.
 */
stabilizer_status stabilizer_apply_file(stabilizer_handle *handle, const char *file_path, char **output);
stabilizer_status stabilizer_apply_text(stabilizer_handle *handle, const char *smt2_text, char **output);

/** Return the last diagnostic string recorded by the handle. */
const char *stabilizer_last_error(const stabilizer_handle *handle);
/** Release strings returned through the C API. */
void stabilizer_free_string(char *value);

#ifdef __cplusplus
}
#endif