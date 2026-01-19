# SRS-REPORT: Report Generation Requirements

**Document ID:** SRS-REPORT  
**Version:** 1.0.0  
**Status:** Release  
**Date:** 2026-01-19  
**Author:** William Murray  
**Traceability:** CH-MATH-001 §8

---

## 1. Purpose

This document specifies requirements for the report generation module, which produces JSON reports and console summaries of pipeline execution results.

---

## 2. Scope

The report module (`report.c`) provides:
- JSON report generation
- Console summary output
- Human-readable stage status display

---

## 3. Requirements

### 3.1 JSON Report Generation

#### REQ-RPT-001: Write JSON Report

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

The function `ch_report_write_json()` SHALL write a JSON report to the specified file path.

**Rationale:** Machine-readable output for tooling.

**Verification:** Unit test `test_report_write_json`

#### REQ-RPT-002: NULL Parameter Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_report_write_json()` SHALL return `CH_ERR_NULL` if `result`, `path`, or `faults` is NULL.

**Rationale:** Defensive programming.

**Verification:** Unit test `test_report_null_safety`

#### REQ-RPT-003: I/O Error Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

The function `ch_report_write_json()` SHALL return `CH_ERR_IO` if the file cannot be opened for writing.

**Rationale:** Graceful error handling.

**Verification:** Unit test with invalid path

#### REQ-RPT-004: Report Version

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

The JSON report SHALL include a `version` field containing `CH_VERSION_STRING`.

**Rationale:** Enables format evolution.

**Verification:** Unit test checking report content

#### REQ-RPT-005: Report Platform

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

The JSON report SHALL include a `platform` field containing `result->platform`.

**Rationale:** Identifies execution platform.

**Verification:** Unit test checking report content

#### REQ-RPT-006: Report Harness Hash

**Priority:** MUST  
**Traceability:** CH-MATH-001 §1.4

The JSON report SHALL include a `harness_hash` field containing the 64-character hex encoding of `result->harness_hash`.

**Rationale:** Traceability to harness binary.

**Verification:** Unit test checking report content

#### REQ-RPT-007: Stages Array

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

The JSON report SHALL include a `stages` array with 7 elements, one for each stage.

**Rationale:** Complete pipeline visibility.

**Verification:** Unit test checking array length

#### REQ-RPT-008: Stage Object Fields

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

Each stage object in the `stages` array SHALL include:
- `stage` — Integer stage index (0-6)
- `name` — Stage name string
- `result` — Result code string
- `hash` — 64-character hex hash
- `duration_us` — Execution time in microseconds

**Rationale:** Complete stage information.

**Verification:** Unit test checking stage structure

#### REQ-RPT-009: Hash Formatting

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

All hashes in the JSON report SHALL be formatted as 64-character lowercase hexadecimal strings.

**Rationale:** Standard hash representation.

**Verification:** Unit test checking format

#### REQ-RPT-010: Summary Fields

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

The JSON report SHALL include:
- `stages_completed` — Count of successful stages
- `all_passed` — Boolean
- `bit_identical` — Boolean

**Rationale:** Quick status overview.

**Verification:** Unit test checking fields

#### REQ-RPT-011: Valid JSON

**Priority:** MUST  
**Traceability:** CH-MATH-001 §8.1

The generated file SHALL be valid JSON that parses without error.

**Rationale:** Interoperability.

**Verification:** Parse with JSON validator

---

### 3.2 Console Summary

#### REQ-RPT-020: Print Summary

**Priority:** MUST  
**Traceability:** Implementation

The function `ch_report_print_summary()` SHALL print a human-readable summary to stdout.

**Rationale:** Immediate feedback to operator.

**Verification:** Manual inspection

#### REQ-RPT-021: Summary Header

**Priority:** MUST  
**Traceability:** Implementation

The summary SHALL include a header showing:
- Harness name and version
- Platform identifier

**Rationale:** Context for the output.

**Verification:** Manual inspection

#### REQ-RPT-022: Stage List

**Priority:** MUST  
**Traceability:** Implementation

The summary SHALL list all 7 stages with:
- Stage index [0-6]
- Stage name
- Status indicator (✓, ✗, or ⊘)
- Result code
- Duration in microseconds

**Rationale:** Per-stage visibility.

**Verification:** Manual inspection

#### REQ-RPT-023: Status Indicators

**Priority:** MUST  
**Traceability:** Implementation

The status indicators SHALL be:
- `✓` — Stage passed (`result == CH_OK`)
- `✗` — Stage failed (error code)
- `⊘` — Stage skipped (`result == CH_ERR_SKIPPED`)

**Rationale:** Quick visual scan.

**Verification:** Manual inspection

#### REQ-RPT-024: Overall Status

**Priority:** MUST  
**Traceability:** Implementation

The summary SHALL include an overall status line:
- "ALL STAGES PASSED ✓" if `all_passed == true`
- "PIPELINE FAILED ✗" if `all_passed == false`

**Rationale:** Clear pass/fail indication.

**Verification:** Manual inspection

#### REQ-RPT-025: Bit-Identity Status

**Priority:** MUST  
**Traceability:** Implementation

If a golden reference was compared, the summary SHALL include:
- "Bit-identical: YES ✓" if `bit_identical == true`
- "Bit-identical: NO ✗" if `bit_identical == false`

**Rationale:** Cross-platform verification result.

**Verification:** Manual inspection

#### REQ-RPT-026: NULL Result Handling

**Priority:** MUST  
**Traceability:** Implementation

The function `ch_report_print_summary()` SHALL return immediately if `result` is NULL.

**Rationale:** Defensive programming.

**Verification:** Unit test

---

## 4. Interface Specification

### 4.1 Function: `ch_report_write_json`

```c
ch_result_t ch_report_write_json(const ch_result_t_full *result,
                                  const ch_golden_t *golden,
                                  const char *path,
                                  ch_fault_flags_t *faults);
```

**Parameters:**
- `result` — Pipeline result (must not be NULL)
- `golden` — Golden reference for comparison (may be NULL)
- `path` — Output file path (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:**
- `CH_OK` — Success
- `CH_ERR_NULL` — NULL required parameter
- `CH_ERR_IO` — File I/O error

**Traceability:** REQ-RPT-001 through REQ-RPT-011

### 4.2 Function: `ch_report_print_summary`

```c
void ch_report_print_summary(const ch_result_t_full *result,
                              const ch_golden_t *golden);
```

**Parameters:**
- `result` — Pipeline result (may be NULL, function returns)
- `golden` — Golden reference (may be NULL)

**Returns:** void

**Traceability:** REQ-RPT-020 through REQ-RPT-026

---

## 5. JSON Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Certifiable Harness Report",
  "type": "object",
  "required": ["version", "platform", "harness_hash", "stages", 
               "stages_completed", "all_passed", "bit_identical"],
  "properties": {
    "version": {
      "type": "string",
      "pattern": "^\\d+\\.\\d+\\.\\d+$"
    },
    "platform": {
      "type": "string",
      "enum": ["x86_64", "aarch64", "riscv64", "unknown"]
    },
    "harness_hash": {
      "type": "string",
      "pattern": "^[0-9a-f]{64}$"
    },
    "stages": {
      "type": "array",
      "minItems": 7,
      "maxItems": 7,
      "items": {
        "type": "object",
        "required": ["stage", "name", "result", "hash", "duration_us"],
        "properties": {
          "stage": {
            "type": "integer",
            "minimum": 0,
            "maximum": 6
          },
          "name": {
            "type": "string",
            "enum": ["data", "training", "quant", "deploy", 
                     "inference", "monitor", "verify"]
          },
          "result": {
            "type": "string"
          },
          "hash": {
            "type": "string",
            "pattern": "^[0-9a-f]{64}$"
          },
          "duration_us": {
            "type": "integer",
            "minimum": 0
          }
        }
      }
    },
    "stages_completed": {
      "type": "integer",
      "minimum": 0,
      "maximum": 7
    },
    "all_passed": {
      "type": "boolean"
    },
    "bit_identical": {
      "type": "boolean"
    }
  }
}
```

---

## 6. Example Output

### 6.1 JSON Report

```json
{
  "version": "1.0.0",
  "platform": "x86_64",
  "harness_hash": "0000000000000000000000000000000000000000000000000000000000000000",
  "stages": [
    {
      "stage": 0,
      "name": "data",
      "result": "OK",
      "hash": "2f0c6228001d125032afbe0163104da5f6ea8a3ef4328de603b5f6af7bee6b1c",
      "duration_us": 4
    },
    {
      "stage": 1,
      "name": "training",
      "result": "OK",
      "hash": "36b34d87459ead09c5349d55a7a187646fc135f75d7e2e8cdd064c9513bf7dfc",
      "duration_us": 4
    },
    ...
  ],
  "stages_completed": 7,
  "all_passed": true,
  "bit_identical": true
}
```

### 6.2 Console Summary

```
═══════════════════════════════════════════════════════════════
  Certifiable Harness — Pipeline Summary
═══════════════════════════════════════════════════════════════

  Platform: x86_64
  Stages completed: 7/7

  [0] data         ✓  (OK, 4 µs)
  [1] training     ✓  (OK, 4 µs)
  [2] quant        ✓  (OK, 3 µs)
  [3] deploy       ✓  (OK, 3 µs)
  [4] inference    ✓  (OK, 3 µs)
  [5] monitor      ✓  (OK, 4 µs)
  [6] verify       ✓  (OK, 8 µs)

  Status: ALL STAGES PASSED ✓
  Bit-identical: YES ✓
═══════════════════════════════════════════════════════════════
```

---

## 7. Traceability Matrix

| Requirement | CH-MATH-001 | Test |
|-------------|-------------|------|
| REQ-RPT-001 | §8.1 | test_report_write_json |
| REQ-RPT-002 | §11.3 | test_report_null_safety |
| REQ-RPT-003 | §11.3 | (integration) |
| REQ-RPT-004 | §8.1 | (content check) |
| REQ-RPT-005 | §8.1 | (content check) |
| REQ-RPT-006 | §1.4 | (content check) |
| REQ-RPT-007 | §8.1 | (content check) |
| REQ-RPT-008 | §8.1 | (content check) |
| REQ-RPT-009 | §8.1 | (content check) |
| REQ-RPT-010 | §8.1 | (content check) |
| REQ-RPT-011 | §8.1 | JSON validation |
| REQ-RPT-020 | — | manual |
| REQ-RPT-021 | — | manual |
| REQ-RPT-022 | — | manual |
| REQ-RPT-023 | — | manual |
| REQ-RPT-024 | — | manual |
| REQ-RPT-025 | — | manual |
| REQ-RPT-026 | — | unit test |

---

## 8. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2026-01-19 | William Murray | Initial release |

---

*Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.*
