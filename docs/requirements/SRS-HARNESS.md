# SRS-HARNESS: Harness Orchestration Requirements

**Document ID:** SRS-HARNESS  
**Version:** 1.0.0  
**Status:** Release  
**Date:** 2026-01-19  
**Author:** William Murray  
**Traceability:** CH-MATH-001 §9

---

## 1. Purpose

This document specifies requirements for the harness orchestration module, which coordinates execution of all seven pipeline stages and produces cryptographic commitments for cross-platform verification.

---

## 2. Scope

The harness module (`harness.c`) provides:
- Default configuration initialization
- Sequential stage execution
- Platform detection
- Harness self-hashing
- Timing measurement

---

## 3. Requirements

### 3.1 Configuration

#### REQ-HAR-001: Default Configuration

**Priority:** MUST  
**Traceability:** CH-MATH-001 §6.3

The function `ch_config_default()` SHALL return a configuration with:
- `num_samples = 1000`
- `batch_size = 32`
- `epochs = 10`
- `verbose = false`
- `generate_golden = false`
- All path pointers set to NULL

**Rationale:** Consistent defaults ensure reproducible results.

**Verification:** Unit test `test_config_default`

---

### 3.2 Pipeline Execution

#### REQ-HAR-002: Sequential Execution

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.1

The function `ch_harness_run()` SHALL execute stages in strict sequential order:
1. data (0)
2. training (1)
3. quant (2)
4. deploy (3)
5. inference (4)
6. monitor (5)
7. verify (6)

**Rationale:** Each stage depends on outputs from previous stages.

**Verification:** Unit test `test_harness_run_basic`

#### REQ-HAR-003: NULL Parameter Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_harness_run()` SHALL return `CH_ERR_NULL` if any of these parameters is NULL:
- `config`
- `result`
- `faults`

**Rationale:** Defensive programming prevents undefined behaviour.

**Verification:** Unit test `test_harness_run_null`

#### REQ-HAR-004: Stage Skipping

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.1

If a stage cannot run due to missing inputs from a previous stage, the harness SHALL:
1. Set `result.stages[s].result = CH_ERR_SKIPPED`
2. Set `result.stages[s].hash` to all zeros
3. Set `result.stages[s].duration_us = 0`
4. Continue to subsequent stages if their inputs are available

**Rationale:** Partial execution may still provide useful diagnostics.

**Verification:** Integration test with forced stage failure

#### REQ-HAR-005: Result Initialization

**Priority:** MUST  
**Traceability:** CH-MATH-001 §7.2

Before executing any stage, `ch_harness_run()` SHALL initialize the result structure to zero using `memset()`.

**Rationale:** Ensures deterministic initial state.

**Verification:** Unit test verification of result fields

#### REQ-HAR-006: Stage Counting

**Priority:** MUST  
**Traceability:** CH-MATH-001 §7.2

The harness SHALL track `stages_completed` as the count of stages where `result == CH_OK`.

**Rationale:** Provides summary metric for pipeline success.

**Verification:** Unit test `test_harness_run_basic`

#### REQ-HAR-007: All Passed Flag

**Priority:** MUST  
**Traceability:** CH-MATH-001 §7.2

The harness SHALL set `all_passed = true` if and only if all seven stages have `result == CH_OK`.

**Rationale:** Single boolean for quick pass/fail determination.

**Verification:** Unit test `test_harness_run_basic`

---

### 3.3 Platform Detection

#### REQ-HAR-008: Platform Identification

**Priority:** MUST  
**Traceability:** CH-MATH-001 §10.2

The function `ch_get_platform()` SHALL return one of:
- `"x86_64"` — Intel/AMD 64-bit
- `"aarch64"` — ARM 64-bit
- `"riscv64"` — RISC-V 64-bit
- `"unknown"` — Unrecognized platform

Detection SHALL use preprocessor macros at compile time.

**Rationale:** Platform identity is recorded in golden reference.

**Verification:** Unit test `test_get_platform`

#### REQ-HAR-009: Platform String Copy

**Priority:** MUST  
**Traceability:** CH-MATH-001 §7.2

The harness SHALL copy the platform string to `result.platform` with proper null termination, not exceeding `CH_PLATFORM_SIZE - 1` characters.

**Rationale:** Prevents buffer overflow.

**Verification:** Unit test verification

---

### 3.4 Harness Self-Hash

#### REQ-HAR-010: Harness Hash Generation

**Priority:** SHOULD  
**Traceability:** CH-MATH-001 §1.4

The function `ch_get_harness_hash()` SHALL compute SHA-256 of the harness binary executable.

**Implementation Note:** On Linux, read `/proc/self/exe`. On other platforms, may return zeros if not implemented.

**Rationale:** Ensures the harness itself is verified (closes observer attack).

**Verification:** Integration test comparing hash of binary

#### REQ-HAR-011: Harness Hash in Result

**Priority:** MUST  
**Traceability:** CH-MATH-001 §1.4

The harness SHALL populate `result.harness_hash` with the output of `ch_get_harness_hash()`.

**Rationale:** Included in reports for traceability.

**Verification:** Unit test verification of field population

---

### 3.5 Timing

#### REQ-HAR-012: Stage Timing

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.1

The harness SHALL measure execution time for each stage using monotonic clock and store in `duration_us`.

**Rationale:** Performance metrics for diagnostics (not part of commitment).

**Verification:** Unit test verification that duration > 0

#### REQ-HAR-013: Timestamp Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.1

The function `ch_get_timestamp()` SHALL return current Unix timestamp (seconds since epoch).

**Rationale:** Recorded in golden reference.

**Verification:** Integration test comparing with system time

#### REQ-HAR-014: Microsecond Timer

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.1

The function `ch_get_time_us()` SHALL return current time in microseconds using `CLOCK_MONOTONIC`.

**Rationale:** High-resolution timing for performance measurement.

**Verification:** Integration test measuring known delay

---

### 3.6 Context Management

#### REQ-HAR-015: Context Initialization

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.2

The harness SHALL initialize a `ch_context_t` structure before stage execution using `ch_context_init()`.

**Rationale:** Ensures clean state for pipeline execution.

**Verification:** Implicit in `test_harness_run_basic`

#### REQ-HAR-016: Context Cleanup

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.2

The harness SHALL call `ch_context_free()` after stage execution to release any allocated resources.

**Rationale:** Prevents resource leaks.

**Verification:** Memory analysis tools (valgrind)

---

## 4. Interface Specification

### 4.1 Function: `ch_config_default`

```c
ch_config_t ch_config_default(void);
```

**Parameters:** None

**Returns:** Default configuration structure

**Traceability:** REQ-HAR-001

### 4.2 Function: `ch_harness_run`

```c
ch_result_t ch_harness_run(const ch_config_t *config,
                           ch_result_t_full *result,
                           ch_fault_flags_t *faults);
```

**Parameters:**
- `config` — Harness configuration (must not be NULL)
- `result` — Output result structure (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:** `CH_OK` on success, error code otherwise

**Traceability:** REQ-HAR-002 through REQ-HAR-007

### 4.3 Function: `ch_get_platform`

```c
const char *ch_get_platform(void);
```

**Parameters:** None

**Returns:** Static string identifying platform

**Traceability:** REQ-HAR-008

### 4.4 Function: `ch_get_harness_hash`

```c
ch_result_t ch_get_harness_hash(uint8_t hash_out[CH_HASH_SIZE],
                                 ch_fault_flags_t *faults);
```

**Parameters:**
- `hash_out` — Output buffer for 32-byte hash
- `faults` — Fault flag accumulator

**Returns:** `CH_OK` on success

**Traceability:** REQ-HAR-010, REQ-HAR-011

### 4.5 Function: `ch_get_timestamp`

```c
uint64_t ch_get_timestamp(void);
```

**Parameters:** None

**Returns:** Unix timestamp in seconds

**Traceability:** REQ-HAR-013

### 4.6 Function: `ch_get_time_us`

```c
uint64_t ch_get_time_us(void);
```

**Parameters:** None

**Returns:** Monotonic time in microseconds

**Traceability:** REQ-HAR-014

---

## 5. Traceability Matrix

| Requirement | CH-MATH-001 | Test |
|-------------|-------------|------|
| REQ-HAR-001 | §6.3 | test_config_default |
| REQ-HAR-002 | §9.1 | test_harness_run_basic |
| REQ-HAR-003 | §11.3 | test_harness_run_null |
| REQ-HAR-004 | §11.1 | (integration) |
| REQ-HAR-005 | §7.2 | test_harness_run_basic |
| REQ-HAR-006 | §7.2 | test_harness_run_basic |
| REQ-HAR-007 | §7.2 | test_harness_run_basic |
| REQ-HAR-008 | §10.2 | test_get_platform |
| REQ-HAR-009 | §7.2 | test_harness_run_basic |
| REQ-HAR-010 | §1.4 | (integration) |
| REQ-HAR-011 | §1.4 | test_harness_run_basic |
| REQ-HAR-012 | §9.1 | test_harness_run_basic |
| REQ-HAR-013 | §4.1 | (integration) |
| REQ-HAR-014 | §9.1 | (integration) |
| REQ-HAR-015 | §9.2 | test_harness_run_basic |
| REQ-HAR-016 | §9.2 | (memory analysis) |

---

## 6. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2026-01-19 | William Murray | Initial release |

---

*Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.*
