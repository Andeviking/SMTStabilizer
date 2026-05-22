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

/**
 * @brief Result status for C API calls.
 */
typedef enum stabilizer_status {
    /** Call succeeded. */
    STABILIZER_STATUS_OK = 0,
    /** One or more arguments were null/invalid. */
    STABILIZER_STATUS_INVALID_ARGUMENT = 1,
    /** Internal parsing or stabilization failure occurred. */
    STABILIZER_STATUS_RUNTIME_ERROR = 2
} stabilizer_status;

/**
 * @brief Allocate a new options object with default values.
 * @return Non-null pointer on success; null on allocation failure.
 */
stabilizer_options *stabilizer_options_create(void);

/**
 * @brief Release an options object created by stabilizer_options_create.
 * @param options Options handle, nullable.
 */
void stabilizer_options_destroy(stabilizer_options *options);

/**
 * @brief Enable or disable parser rewrite normalization.
 * @param options Options handle, must not be null.
 * @param value New rewrite flag.
 */
void stabilizer_options_set_rewrite(stabilizer_options *options, bool value);

/**
 * @brief Query parser rewrite normalization flag.
 * @param options Options handle, must not be null.
 * @return Current rewrite flag. Returns false when options is null.
 */
bool stabilizer_options_get_rewrite(const stabilizer_options *options);

/**
 * @brief Enable or disable satisfiability checking in the public API.
 * @param options Options handle, must not be null.
 * @param value New check-sat flag.
 */
void stabilizer_options_set_check_sat(stabilizer_options *options, bool value);

/**
 * @brief Query satisfiability checking flag.
 * @param options Options handle, must not be null.
 * @return Current flag. Returns false when options is null.
 */
bool stabilizer_options_get_check_sat(const stabilizer_options *options);

/**
 * @brief Select the solver backend used for satisfiability checking.
 * @param options Options handle, must not be null.
 * @param solver Solver name. Currently only "bitwuzla" is accepted.
 */
void stabilizer_options_set_solver(stabilizer_options *options, const char *solver);

/**
 * @brief Query the solver backend name.
 * @param options Options handle, must not be null.
 * @return Solver backend name owned by the options object; returns "" when options is null.
 */
const char *stabilizer_options_get_solver(const stabilizer_options *options);

/**
 * @brief Create a stabilizer instance.
 * @param options Optional options pointer. When null, defaults are used.
 * @return Non-null handle on success; null on allocation failure.
 */
stabilizer_handle *stabilizer_create(const stabilizer_options *options);

/**
 * @brief Destroy a stabilizer instance and associated internal state.
 * @param handle Stabilizer handle, nullable.
 */
void stabilizer_destroy(stabilizer_handle *handle);

/**
 * @brief Update rewrite normalization on a live stabilizer.
 * @param handle Stabilizer handle, must not be null.
 * @param value New rewrite flag.
 */
void stabilizer_set_rewrite(stabilizer_handle *handle, bool value);

/**
 * @brief Query rewrite normalization on a live stabilizer.
 * @param handle Stabilizer handle, must not be null.
 * @return Current rewrite flag. Returns false when handle is null.
 */
bool stabilizer_get_rewrite(const stabilizer_handle *handle);

/**
 * @brief Update satisfiability checking on a live stabilizer.
 * @param handle Stabilizer handle, must not be null.
 * @param value New check-sat flag.
 */
void stabilizer_set_check_sat(stabilizer_handle *handle, bool value);

/**
 * @brief Query satisfiability checking on a live stabilizer.
 * @param handle Stabilizer handle, must not be null.
 * @return Current flag. Returns false when handle is null.
 */
bool stabilizer_get_check_sat(const stabilizer_handle *handle);

/**
 * @brief Update the solver backend on a live stabilizer.
 * @param handle Stabilizer handle, must not be null.
 * @param solver Solver name. Currently only "bitwuzla" is accepted.
 */
void stabilizer_set_solver(stabilizer_handle *handle, const char *solver);

/**
 * @brief Query the solver backend on a live stabilizer.
 * @param handle Stabilizer handle, must not be null.
 * @return Solver backend name owned by the handle; returns "" when handle is null.
 */
const char *stabilizer_get_solver(const stabilizer_handle *handle);

/**
 * @brief Apply stabilization pipeline to an SMT-LIB2 file.
 * @param handle Stabilizer handle, must not be null.
 * @param file_path Input file path, must not be null/empty.
 * @param output Output pointer for heap-allocated UTF-8 result string.
 *
 * On success, `*output` receives memory that must be released with
 * stabilizer_free_string(). The handle remains valid after the call. When
 * check-sat is enabled, the output string is the solver status instead of
 * stabilized SMT2 text.
 *
 * @return STABILIZER_STATUS_OK on success;
 * STABILIZER_STATUS_INVALID_ARGUMENT for null/invalid inputs;
 * STABILIZER_STATUS_RUNTIME_ERROR on parser/kernel failures.
 */
stabilizer_status stabilizer_apply_file(stabilizer_handle *handle, const char *file_path, char **output);

/**
 * @brief Apply stabilization pipeline to SMT-LIB2 text.
 * @param handle Stabilizer handle, must not be null.
 * @param smt2_text Full SMT-LIB2 script text, must not be null/empty.
 * @param output Output pointer for heap-allocated UTF-8 result string.
 *
 * On success, `*output` receives memory that must be released with
 * stabilizer_free_string(). The handle remains valid after the call. When
 * check-sat is enabled, the output string is the solver status instead of
 * stabilized SMT2 text.
 *
 * @return STABILIZER_STATUS_OK on success;
 * STABILIZER_STATUS_INVALID_ARGUMENT for null/invalid inputs;
 * STABILIZER_STATUS_RUNTIME_ERROR on parser/kernel failures.
 */
stabilizer_status stabilizer_apply_text(stabilizer_handle *handle, const char *smt2_text, char **output);

/**
 * @brief Return the last diagnostic message associated with a handle.
 * @param handle Stabilizer handle.
 * @return Null-terminated string owned by the handle; do not free.
 */
const char *stabilizer_last_error(const stabilizer_handle *handle);

/**
 * @brief Release strings returned through stabilizer_apply_*.
 * @param value String buffer previously returned by this API, nullable.
 */
void stabilizer_free_string(char *value);

#ifdef __cplusplus
}
#endif