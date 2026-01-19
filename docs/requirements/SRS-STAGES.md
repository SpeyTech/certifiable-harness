# SRS-STAGES: Stage Wrapper Requirements

**Document ID:** SRS-STAGES  
**Version:** 1.0.0  
**Status:** Release  
**Date:** 2026-01-19  
**Author:** William Murray  
**Traceability:** CH-MATH-001 §3

---

## 1. Purpose

This document specifies requirements for the stage wrapper module, which provides the interface between the harness orchestrator and the seven certifiable-* libraries.

---

## 2. Scope

The stages module (`stages.c`) provides:
- Context initialization and cleanup
- Seven stage wrapper functions
- Stub implementations for standalone testing
- Wired implementations for full integration

---

## 3. Requirements

### 3.1 Context Management

#### REQ-STG-001: Context Initialization

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.2

The function `ch_context_init()` SHALL zero-initialize the entire `ch_context_t` structure using `memset()`.

**Rationale:** Ensures clean state for pipeline execution.

**Verification:** Unit test `test_context_init`

#### REQ-STG-002: Context Cleanup

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.2

The function `ch_context_free()` SHALL release any dynamically allocated resources and zero the structure.

**Rationale:** Prevents resource leaks and dangling pointers.

**Verification:** Memory analysis (valgrind)

#### REQ-STG-003: Context NULL Safety

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

Both `ch_context_init()` and `ch_context_free()` SHALL handle NULL input gracefully (no-op).

**Rationale:** Defensive programming.

**Verification:** Unit test

---

### 3.2 Stage Function Common Requirements

#### REQ-STG-004: NULL Parameter Handling

**Priority:** MUST  
**Traceability:** CH-MATH-001 §11.3

All stage functions SHALL return `CH_ERR_NULL` if any of these parameters is NULL:
- `config`
- `ctx`
- `result`
- `faults`

**Rationale:** Defensive programming.

**Verification:** Unit test `test_stage_null_safety`

#### REQ-STG-005: Stage Identifier

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.1

Each stage function SHALL set `result->stage` to the appropriate `ch_stage_t` value.

**Rationale:** Self-documenting result.

**Verification:** Unit test `test_stage_data`

#### REQ-STG-006: Validity Flag

**Priority:** MUST  
**Traceability:** CH-MATH-001 §9.2

On successful execution, each stage function SHALL set `ctx->stage_valid[stage] = true`.

**Rationale:** Enables dependency checking.

**Verification:** Unit test `test_stage_data`

#### REQ-STG-007: Commitment Storage

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3

Each stage function SHALL copy its commitment hash to both:
- `result->hash`
- The appropriate `ctx->*_hash` field

**Rationale:** Makes commitment available to dependent stages.

**Verification:** Unit test `test_stage_data`

---

### 3.3 Stage 0: Data

#### REQ-STG-010: Data Stage Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3.1

The function `ch_stage_data()` SHALL:
1. Load test dataset (if `config->data_path` provided)
2. Create batches according to `config->batch_size`
3. Compute Merkle root of all batch hashes
4. Store Merkle root in `result->hash` and `ctx->data_merkle_root`

**Commitment:** M_data = MerkleRoot(H_batch[0..n-1])

**Rationale:** Establishes data provenance baseline.

**Verification:** Unit test `test_stage_data`

#### REQ-STG-011: Data Stage Independence

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.2

The data stage SHALL have no dependencies on other stages.

**Rationale:** First stage in pipeline.

**Verification:** Unit test running data stage in isolation

---

### 3.4 Stage 1: Training

#### REQ-STG-020: Training Stage Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3.2

The function `ch_stage_training()` SHALL:
1. Check that data stage completed (`ctx->stage_valid[CH_STAGE_DATA]`)
2. Run training loop for `config->epochs` epochs
3. Compute training chain hash including data commitment
4. Store hash in `result->hash` and `ctx->training_hash`

**Commitment:** H_train = H(data_merkle_root || epoch_hashes)

**Rationale:** Links training to data provenance.

**Verification:** Unit test `test_all_stages_run`

#### REQ-STG-021: Training Stage Dependency

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.2

The training stage SHALL return `CH_ERR_SKIPPED` if `ctx->stage_valid[CH_STAGE_DATA] == false`.

**Rationale:** Cannot train without data.

**Verification:** Integration test

---

### 3.5 Stage 2: Quantization

#### REQ-STG-030: Quant Stage Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3.3

The function `ch_stage_quant()` SHALL:
1. Check that training stage completed
2. Quantize weights from FP32 to Q16.16
3. Generate quantization certificate
4. Store certificate hash in `result->hash` and `ctx->quant_hash`

**Commitment:** H_cert = H(certificate)

**Rationale:** Certifies quantization correctness.

**Verification:** Unit test `test_all_stages_run`

#### REQ-STG-031: Quant Stage Dependency

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.2

The quant stage SHALL return `CH_ERR_SKIPPED` if `ctx->stage_valid[CH_STAGE_TRAINING] == false`.

**Rationale:** Cannot quantize without trained weights.

**Verification:** Integration test

---

### 3.6 Stage 3: Deploy

#### REQ-STG-040: Deploy Stage Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3.4

The function `ch_stage_deploy()` SHALL:
1. Check that quant stage completed
2. Package quantized model into CBF bundle
3. Compute attestation Merkle tree
4. Store attestation root in `result->hash` and `ctx->deploy_root`

**Commitment:** R = MerkleRoot(bundle_files)

**Rationale:** Creates deployable, verifiable package.

**Verification:** Unit test `test_all_stages_run`

#### REQ-STG-041: Deploy Stage Dependency

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.2

The deploy stage SHALL return `CH_ERR_SKIPPED` if `ctx->stage_valid[CH_STAGE_QUANT] == false`.

**Rationale:** Cannot deploy without quantized model.

**Verification:** Integration test

---

### 3.7 Stage 4: Inference

#### REQ-STG-050: Inference Stage Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3.5

The function `ch_stage_inference()` SHALL:
1. Check that deploy stage completed
2. Load model from bundle
3. Run inference on test inputs
4. Compute hash of all predictions
5. Store hash in `result->hash` and `ctx->inference_hash`

**Commitment:** H_pred = H(predictions[0..n-1])

**Rationale:** Captures inference outputs.

**Verification:** Unit test `test_all_stages_run`

#### REQ-STG-051: Inference Stage Dependency

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.2

The inference stage SHALL return `CH_ERR_SKIPPED` if `ctx->stage_valid[CH_STAGE_DEPLOY] == false`.

**Rationale:** Cannot infer without deployed model.

**Verification:** Integration test

---

### 3.8 Stage 5: Monitor

#### REQ-STG-060: Monitor Stage Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3.6

The function `ch_stage_monitor()` SHALL:
1. Check that deploy and inference stages completed
2. Load COE policy (if `config->policy_path` provided)
3. Initialize ledger with genesis binding to deploy root
4. Record monitoring events
5. Compute final ledger digest
6. Store digest in `result->hash` and `ctx->monitor_digest`

**Commitment:** L_n = H(L_{n-1} || event_n)

**Rationale:** Creates audit trail.

**Verification:** Unit test `test_all_stages_run`

#### REQ-STG-061: Monitor Stage Dependencies

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.2

The monitor stage SHALL return `CH_ERR_SKIPPED` if either:
- `ctx->stage_valid[CH_STAGE_DEPLOY] == false`
- `ctx->stage_valid[CH_STAGE_INFERENCE] == false`

**Rationale:** Monitor needs both bundle and predictions.

**Verification:** Integration test

---

### 3.9 Stage 6: Verify

#### REQ-STG-070: Verify Stage Function

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3.7

The function `ch_stage_verify()` SHALL:
1. Check that all previous stages completed
2. Verify all six bindings between stages
3. Generate verification report
4. Compute report hash
5. Store hash in `result->hash` and `ctx->verify_hash`

**Commitment:** H_rpt = H(report)

**Rationale:** Final verification of entire pipeline.

**Verification:** Unit test `test_all_stages_run`

#### REQ-STG-071: Verify Stage Dependencies

**Priority:** MUST  
**Traceability:** CH-MATH-001 §2.2

The verify stage SHALL return `CH_ERR_SKIPPED` if any of stages 0-5 has `stage_valid == false`.

**Rationale:** Cannot verify incomplete pipeline.

**Verification:** Integration test

---

### 3.10 Stub vs Wired Implementation

#### REQ-STG-080: Compile-Time Selection

**Priority:** MUST  
**Traceability:** Implementation detail

Each stage SHALL support compile-time selection between stub and wired implementation via preprocessor macros:
- `CH_LINK_CERTIFIABLE_DATA`
- `CH_LINK_CERTIFIABLE_TRAINING`
- `CH_LINK_CERTIFIABLE_QUANT`
- `CH_LINK_CERTIFIABLE_DEPLOY`
- `CH_LINK_CERTIFIABLE_INFERENCE`
- `CH_LINK_CERTIFIABLE_MONITOR`
- `CH_LINK_CERTIFIABLE_VERIFY`

Default: 0 (stub)

**Rationale:** Enables standalone testing without all libraries.

**Verification:** Build with and without flags

#### REQ-STG-081: Stub Determinism

**Priority:** MUST  
**Traceability:** CH-MATH-001 §12

Stub implementations SHALL produce deterministic hashes based solely on:
- Stage identifier
- Configuration parameters
- Previous stage commitments

**Rationale:** Enables cross-platform testing without real libraries.

**Verification:** Cross-platform comparison (proven: Linux ↔ macOS)

#### REQ-STG-082: Stub Chaining

**Priority:** MUST  
**Traceability:** CH-MATH-001 §3

Stub implementations SHALL include previous stage commitments in their hash preimage to maintain chain integrity.

**Rationale:** Simulates real binding behaviour.

**Verification:** Code review

---

## 4. Interface Specification

### 4.1 Function: `ch_context_init`

```c
void ch_context_init(ch_context_t *ctx);
```

**Parameters:**
- `ctx` — Context to initialize (may be NULL)

**Returns:** void

**Traceability:** REQ-STG-001, REQ-STG-003

### 4.2 Function: `ch_context_free`

```c
void ch_context_free(ch_context_t *ctx);
```

**Parameters:**
- `ctx` — Context to free (may be NULL)

**Returns:** void

**Traceability:** REQ-STG-002, REQ-STG-003

### 4.3 Stage Functions

All stage functions share this signature:

```c
ch_result_t ch_stage_<name>(const ch_config_t *config,
                            ch_context_t *ctx,
                            ch_stage_result_t *result,
                            ch_fault_flags_t *faults);
```

**Parameters:**
- `config` — Harness configuration (must not be NULL)
- `ctx` — Pipeline context (must not be NULL)
- `result` — Output stage result (must not be NULL)
- `faults` — Fault flag accumulator (must not be NULL)

**Returns:**
- `CH_OK` — Success
- `CH_ERR_NULL` — NULL parameter
- `CH_ERR_SKIPPED` — Missing dependencies
- `CH_ERR_STAGE` — Stage execution failed

**Functions:**
- `ch_stage_data()` — REQ-STG-010, REQ-STG-011
- `ch_stage_training()` — REQ-STG-020, REQ-STG-021
- `ch_stage_quant()` — REQ-STG-030, REQ-STG-031
- `ch_stage_deploy()` — REQ-STG-040, REQ-STG-041
- `ch_stage_inference()` — REQ-STG-050, REQ-STG-051
- `ch_stage_monitor()` — REQ-STG-060, REQ-STG-061
- `ch_stage_verify()` — REQ-STG-070, REQ-STG-071

---

## 5. Dependency Graph

```
┌─────────┐
│  data   │ (stage 0)
└────┬────┘
     │
     ▼
┌─────────┐
│training │ (stage 1)
└────┬────┘
     │
     ▼
┌─────────┐
│  quant  │ (stage 2)
└────┬────┘
     │
     ▼
┌─────────┐
│ deploy  │ (stage 3)
└────┬────┘
     │
     ├─────────────────┐
     ▼                 ▼
┌─────────┐      ┌─────────┐
│inference│      │         │
│(stage 4)│      │         │
└────┬────┘      │         │
     │           │         │
     └─────┬─────┘         │
           ▼               │
     ┌─────────┐           │
     │ monitor │ ◄─────────┘
     │(stage 5)│
     └────┬────┘
          │
          ▼
     ┌─────────┐
     │ verify  │ (stage 6) — requires all 0-5
     └─────────┘
```

---

## 6. Traceability Matrix

| Requirement | CH-MATH-001 | Test |
|-------------|-------------|------|
| REQ-STG-001 | §9.2 | test_context_init |
| REQ-STG-002 | §9.2 | (memory analysis) |
| REQ-STG-003 | §11.3 | (code review) |
| REQ-STG-004 | §11.3 | test_stage_null_safety |
| REQ-STG-005 | §2.1 | test_stage_data |
| REQ-STG-006 | §9.2 | test_stage_data |
| REQ-STG-007 | §3 | test_stage_data |
| REQ-STG-010 | §3.1 | test_stage_data |
| REQ-STG-011 | §2.2 | test_stage_data |
| REQ-STG-020 | §3.2 | test_all_stages_run |
| REQ-STG-021 | §2.2 | (integration) |
| REQ-STG-030 | §3.3 | test_all_stages_run |
| REQ-STG-031 | §2.2 | (integration) |
| REQ-STG-040 | §3.4 | test_all_stages_run |
| REQ-STG-041 | §2.2 | (integration) |
| REQ-STG-050 | §3.5 | test_all_stages_run |
| REQ-STG-051 | §2.2 | (integration) |
| REQ-STG-060 | §3.6 | test_all_stages_run |
| REQ-STG-061 | §2.2 | (integration) |
| REQ-STG-070 | §3.7 | test_all_stages_run |
| REQ-STG-071 | §2.2 | (integration) |
| REQ-STG-080 | — | (build test) |
| REQ-STG-081 | §12 | cross-platform test |
| REQ-STG-082 | §3 | code review |

---

## 7. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2026-01-19 | William Murray | Initial release |

---

*Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.*
