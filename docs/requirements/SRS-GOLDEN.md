# SRS-GOLDEN: Golden Reference Requirements

**Document ID:** SRS-GOLDEN  
**Version:** 1.0.0  
**Status:** Release  
**Date:** 2026-01-19  
**Author:** William Murray  
**Traceability:** CH-MATH-001 §4, §5

---

## 1. Purpose

This document specifies requirements for the golden reference module, which handles loading, saving, generating, and comparing golden reference files for cross-platform bit-identity verification.

---

## 2. Scope

The golden module (`golden.c`) provides:
- Golden file loading with validation
- Golden file saving with integrity hash
- Golden generation from pipeline results
- Bit-identity comparison

---

## 3. Requirements

### 3.1 Golden Loading

#### REQ-GLD-001: Load from File

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.3

The function `ch_golden_load()` SHALL read a 368-byte golden reference file from the specified path.

**Rationale:** Enables cross-platform comparison.

**Verification:** Unit test `test_golden_load`

#### REQ-GLD-002: NULL Parameter Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_golden_load()` SHALL return `CH_ERR_NULL` if any parameter is NULL.

**Rationale:** Defensive programming.

**Verification:** Unit test `test_golden_null_safety`

#### REQ-GLD-003: Magic Validation

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.1

The function `ch_golden_load()` SHALL verify that `magic == 0x52474843` ("CHGR") and return `CH_ERR_PARSE` if invalid.

**Rationale:** Detects corrupted or wrong file type.

**Verification:** Unit test with invalid magic

#### REQ-GLD-004: Version Validation

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.1

The function `ch_golden_load()` SHALL verify that `version == 1` and return `CH_ERR_PARSE` if invalid.

**Rationale:** Enables future format evolution.

**Verification:** Unit test with invalid version

#### REQ-GLD-005: File Hash Verification

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.2

The function `ch_golden_load()` SHALL:
1. Compute SHA-256 of bytes 0x00–0x14F
2. Compare with stored `file_hash` at offset 0x150
3. Return `CH_ERR_GOLDEN` if mismatch

**Rationale:** Detects tampering or corruption.

**Verification:** Unit test with corrupted file

#### REQ-GLD-006: I/O Error Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_golden_load()` SHALL return `CH_ERR_IO` if:
- File cannot be opened
- File read returns fewer than 368 bytes

**Rationale:** Graceful handling of filesystem errors.

**Verification:** Unit test with missing file

---

### 3.2 Golden Saving

#### REQ-GLD-007: Save to File

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.4

The function `ch_golden_save()` SHALL write exactly 368 bytes to the specified path.

**Rationale:** Fixed-size format ensures compatibility.

**Verification:** Unit test checking file size

#### REQ-GLD-008: Save NULL Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_golden_save()` SHALL return `CH_ERR_NULL` if `golden` or `path` is NULL.

**Rationale:** Defensive programming.

**Verification:** Unit test `test_golden_null_safety`

#### REQ-GLD-009: Save I/O Error

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_golden_save()` SHALL return `CH_ERR_IO` if the file cannot be written.

**Rationale:** Graceful handling of filesystem errors.

**Verification:** Unit test with read-only path

---

### 3.3 Golden Generation

#### REQ-GLD-010: Generate from Result

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.4

The function `ch_golden_generate()` SHALL populate a `ch_golden_t` structure from:
- Pipeline result (`ch_result_t_full`)
- Configuration (`ch_config_t`)

**Rationale:** Creates golden reference from successful run.

**Verification:** Unit test `test_golden_generate`

#### REQ-GLD-011: Set Magic and Version

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.1

The function `ch_golden_generate()` SHALL set:
- `magic = 0x52474843`
- `version = 1`

**Rationale:** Identifies file format.

**Verification:** Unit test `test_golden_generate`

#### REQ-GLD-012: Copy Platform

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.1

The function `ch_golden_generate()` SHALL copy `result.platform` to `golden.platform`.

**Rationale:** Records source platform.

**Verification:** Unit test `test_golden_generate`

#### REQ-GLD-013: Set Timestamp

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.1

The function `ch_golden_generate()` SHALL set `timestamp` to current Unix time via `ch_get_timestamp()`.

**Rationale:** Records generation time.

**Verification:** Integration test

#### REQ-GLD-014: Copy Harness Hash

**Priority:** MUST  
**Traceability:** CH-MATH-001 §1.4

The function `ch_golden_generate()` SHALL copy `result.harness_hash` to `golden.harness_hash`.

**Rationale:** Binds golden to specific harness binary.

**Verification:** Unit test `test_golden_generate`

#### REQ-GLD-015: Copy Stage Commitments

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3

The function `ch_golden_generate()` SHALL copy all seven stage hashes from `result.stages[s].hash` to `golden.commitments[s]`.

**Rationale:** Records cryptographic commitments.

**Verification:** Unit test `test_golden_generate`

#### REQ-GLD-016: Compute File Hash

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.2

The function `ch_golden_generate()` SHALL:
1. Populate all fields except `file_hash`
2. Compute `file_hash = SHA256(golden[0:0x150])`

**Rationale:** Enables integrity verification on load.

**Verification:** Unit test verifying round-trip

---

### 3.4 Golden Comparison

#### REQ-GLD-017: Compare Results

**Priority:** MUST  
**Traceability:** CH-MATH-001 §5.1

The function `ch_golden_compare()` SHALL compare all seven stage commitments between result and golden.

**Rationale:** Determines bit-identity.

**Verification:** Unit test `test_golden_compare_identical`

#### REQ-GLD-018: Bit-Identical Output

**Priority:** MUST  
**Traceability:** CH-MATH-001 §5.1

The function `ch_golden_compare()` SHALL set `*bit_identical = true` if and only if all seven stage hashes match.

**Rationale:** Single boolean for quick verification.

**Verification:** Unit test `test_golden_compare_identical`

#### REQ-GLD-019: First Mismatch Detection

**Priority:** MUST  
**Traceability:** CH-MATH-001 §5.2

The function `ch_golden_compare()` SHALL set `*first_mismatch` to:
- `-1` if all hashes match
- Index of first mismatched stage (0–6) if any differ

**Rationale:** Pinpoints where determinism broke down.

**Verification:** Unit test with forced mismatch

#### REQ-GLD-020: Constant-Time Comparison

**Priority:** SHOULD  
**Traceability:** Security best practice

Hash comparisons SHOULD use constant-time operations to prevent timing attacks.

**Rationale:** Security hardening.

**Verification:** Code review

#### REQ-GLD-021: Compare NULL Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_golden_compare()` SHALL return `CH_ERR_NULL` if any parameter is NULL.

**Rationale:** Defensive programming.

**Verification:** Unit test `test_golden_null_safety`

---

### 3.5 File Hash Computation

#### REQ-GLD-022: Compute Hash

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.2

The function `ch_golden_compute_hash()` SHALL compute SHA-256 over bytes 0x00 through 0x14F of the golden structure (336 bytes, excluding `file_hash`).

**Rationale:** Self-integrity verification.

**Verification:** Unit test verifying hash matches after round-trip

#### REQ-GLD-023: Hash Preimage

**Priority:** MUST  
**Traceability:** CH-MATH-001 §4.2

The hash preimage SHALL be:
```
magic || version || platform || timestamp || config_hash || harness_hash || commitments[0..6]
```

Total: 336 bytes

**Rationale:** Deterministic hash input.

**Verification:** Manual verification of offset calculation

---

## 4. Interface Specification

### 4.1 Function: `ch_golden_load`

```c
ch_result_t ch_golden_load(const char *path,
                            ch_golden_t *golden,
                            ch_fault_flags_t *faults);
```

**Parameters:**
- `path` — Path to golden file (must not be NULL)
- `golden` — Output golden structure (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:**
- `CH_OK` — Success
- `CH_ERR_NULL` — NULL parameter
- `CH_ERR_IO` — File I/O error
- `CH_ERR_PARSE` — Invalid magic or version
- `CH_ERR_GOLDEN` — File hash mismatch

**Traceability:** REQ-GLD-001 through REQ-GLD-006

### 4.2 Function: `ch_golden_save`

```c
ch_result_t ch_golden_save(const ch_golden_t *golden,
                            const char *path,
                            ch_fault_flags_t *faults);
```

**Parameters:**
- `golden` — Golden structure to save (must not be NULL)
- `path` — Output path (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:**
- `CH_OK` — Success
- `CH_ERR_NULL` — NULL parameter
- `CH_ERR_IO` — File I/O error

**Traceability:** REQ-GLD-007 through REQ-GLD-009

### 4.3 Function: `ch_golden_generate`

```c
ch_result_t ch_golden_generate(const ch_result_t_full *result,
                                const ch_config_t *config,
                                ch_golden_t *golden,
                                ch_fault_flags_t *faults);
```

**Parameters:**
- `result` — Pipeline result (must not be NULL)
- `config` — Configuration used (must not be NULL)
- `golden` — Output golden structure (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:**
- `CH_OK` — Success
- `CH_ERR_NULL` — NULL parameter

**Traceability:** REQ-GLD-010 through REQ-GLD-016

### 4.4 Function: `ch_golden_compare`

```c
ch_result_t ch_golden_compare(const ch_result_t_full *result,
                               const ch_golden_t *golden,
                               bool *bit_identical,
                               int *first_mismatch,
                               ch_fault_flags_t *faults);
```

**Parameters:**
- `result` — Pipeline result (must not be NULL)
- `golden` — Golden reference (must not be NULL)
- `bit_identical` — Output: true if all match (must not be NULL)
- `first_mismatch` — Output: first mismatch index or -1 (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:**
- `CH_OK` — Success
- `CH_ERR_NULL` — NULL parameter

**Traceability:** REQ-GLD-017 through REQ-GLD-021

### 4.5 Function: `ch_golden_compute_hash`

```c
ch_result_t ch_golden_compute_hash(const ch_golden_t *golden,
                                    uint8_t hash_out[CH_HASH_SIZE],
                                    ch_fault_flags_t *faults);
```

**Parameters:**
- `golden` — Golden structure (must not be NULL)
- `hash_out` — Output buffer for 32-byte hash (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:**
- `CH_OK` — Success
- `CH_ERR_NULL` — NULL parameter

**Traceability:** REQ-GLD-022, REQ-GLD-023

---

## 5. Traceability Matrix

| Requirement | CH-MATH-001 | Test |
|-------------|-------------|------|
| REQ-GLD-001 | §4.3 | test_golden_load |
| REQ-GLD-002 | §11.3 | test_golden_null_safety |
| REQ-GLD-003 | §4.1 | (integration) |
| REQ-GLD-004 | §4.1 | (integration) |
| REQ-GLD-005 | §4.2 | (integration) |
| REQ-GLD-006 | §11.3 | (integration) |
| REQ-GLD-007 | §4.4 | (integration) |
| REQ-GLD-008 | §11.3 | test_golden_null_safety |
| REQ-GLD-009 | §11.3 | (integration) |
| REQ-GLD-010 | §4.4 | test_golden_generate |
| REQ-GLD-011 | §4.1 | test_golden_generate |
| REQ-GLD-012 | §4.1 | test_golden_generate |
| REQ-GLD-013 | §4.1 | (integration) |
| REQ-GLD-014 | §1.4 | test_golden_generate |
| REQ-GLD-015 | §3 | test_golden_generate |
| REQ-GLD-016 | §4.2 | test_golden_generate |
| REQ-GLD-017 | §5.1 | test_golden_compare_identical |
| REQ-GLD-018 | §5.1 | test_golden_compare_identical |
| REQ-GLD-019 | §5.2 | (integration) |
| REQ-GLD-020 | — | code review |
| REQ-GLD-021 | §11.3 | test_golden_null_safety |
| REQ-GLD-022 | §4.2 | (integration) |
| REQ-GLD-023 | §4.2 | code review |

---

## 6. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2026-01-19 | William Murray | Initial release |

---

*Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.*
