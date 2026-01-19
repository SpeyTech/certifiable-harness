# CH-MATH-001: Certifiable Harness Mathematical Specification

**Document ID:** CH-MATH-001  
**Version:** 1.0.0  
**Status:** Draft  
**Author:** William Murray  
**Date:** January 19, 2026  

**Traceability:** This document provides the mathematical foundation for certifiable-harness. All code in `src/` SHALL be a literal transcription of the mathematics herein.

---

## §1 Introduction

### §1.1 Purpose

This document specifies the mathematical foundations for the certifiable-harness end-to-end test harness. The harness orchestrates the complete certifiable-* pipeline and verifies **bit-identity** across platforms.

### §1.2 Scope

The harness:
1. Executes all 7 pipeline stages in sequence
2. Captures cryptographic commitments at each stage
3. Generates golden reference files
4. Compares results across platforms
5. Produces verification reports

### §1.3 Core Theorem

**Bit-Identity Theorem:** For any two DVM-compliant platforms A and B executing the same pipeline with identical inputs:

```
∀ stage s ∈ {data, training, quant, deploy, inference, monitor, verify}:
    H_s(A) = H_s(B)
```

Where `H_s(P)` is the cryptographic commitment produced by stage `s` on platform `P`.

**Corollary:** If bit-identity holds, the pipeline is deterministic and certifiable.

### §1.4 Harness Self-Hash

The certifiable-harness binary SHALL compute its own SHA-256 hash (`H_H`) and include it in:
- The verification report
- Golden reference metadata

```
H_H = SHA256(harness_binary)
```

This closes the "observer attack" — ensuring the harness itself cannot be modified to mask nondeterminism.

**Traceability:** Mirrors CV-MATH-001 §10.8 (Verifier Hash)

### §1.5 Notation

| Symbol | Meaning |
|--------|---------|
| `H_s` | Stage commitment hash (32 bytes) |
| `H_H` | Harness binary hash (32 bytes) |
| `G` | Golden reference |
| `P` | Platform identifier |
| `∥` | Concatenation |
| `SHA256(x)` | SHA-256 hash of x |

---

## §2 Pipeline Stages

### §2.1 Stage Enumeration

```
STAGE_DATA      = 0    /* Data ingestion and batching */
STAGE_TRAINING  = 1    /* Model training */
STAGE_QUANT     = 2    /* FP32 → Q16.16 quantization */
STAGE_DEPLOY    = 3    /* Bundle packaging */
STAGE_INFERENCE = 4    /* Forward pass execution */
STAGE_MONITOR   = 5    /* Runtime monitoring */
STAGE_VERIFY    = 6    /* Formal verification */

STAGE_COUNT = 7
```

### §2.2 Stage Dependencies

```
STAGE_DATA ────────────────┐
                           ▼
STAGE_TRAINING ◄───────────┘
       │
       ▼
STAGE_QUANT
       │
       ▼
STAGE_DEPLOY ──────────────┐
       │                   │
       ▼                   ▼
STAGE_INFERENCE      STAGE_MONITOR
       │                   │
       └───────────────────┘
                 │
                 ▼
          STAGE_VERIFY
```

### §2.3 Stage Execution Order

Stages MUST execute in numerical order: 0, 1, 2, 3, 4, 5, 6.

Each stage receives outputs from previous stages as inputs.

---

## §3 Stage Commitments

### §3.1 Data Stage Commitment

**Input:** Raw dataset (CSV or binary)

**Output:** Merkle root of batches

```
M_data = MERKLE_ROOT(batches)

Where:
    batches = BATCH_ALL(dataset, batch_size, seed)
    BATCH_ALL produces deterministic batch assignment
```

**Commitment:**
```
H_data = M_data
```

**Traceability:** certifiable-data, CT-MATH-001 §3

---

### §3.2 Training Stage Commitment

**Input:** Batched data from Stage 0, hyperparameters

**Output:** Training chain hash

```
H_train = TRAINING_CHAIN_FINAL(epochs, batches, hyperparams)

Where:
    TRAINING_CHAIN_FINAL is the hash after all epochs complete
    Each step: H_t = SHA256("CT:TRAINING:v1" ∥ H_{t-1} ∥ epoch ∥ batch ∥ H_weights)
```

**Commitment:**
```
H_training = H_train
```

**Traceability:** certifiable-training, CT-MATH-001 §4

---

### §3.3 Quantization Stage Commitment

**Input:** Trained FP32 weights from Stage 1

**Output:** Quantization certificate hash

```
cert = QUANTIZE(weights_fp32, config)
H_quant = SHA256("CQ:CERT:v1" ∥ serialize(cert))

Where:
    cert contains: model_hash, quant_hash, max_error, mean_error, layer_errors
```

**Commitment:**
```
H_quant = H(cert)
```

**Traceability:** certifiable-quant, CQ-MATH-001 §5

---

### §3.4 Deploy Stage Commitment

**Input:** Quantized weights, certificate from Stages 1-2

**Output:** Bundle attestation root

```
R = ATTESTATION_ROOT(manifest, weights_q16, cert, config)

Where:
    manifest = JCS_CANONICAL(manifest_json)
    R = MERKLE_ROOT([H_manifest, H_weights, H_cert, H_config])
```

**Commitment:**
```
H_deploy = R
```

**Traceability:** certifiable-deploy, CD-MATH-001 §6

---

### §3.5 Inference Stage Commitment

**Input:** Bundle from Stage 3, test inputs

**Output:** Predictions hash

```
predictions = INFERENCE_ALL(bundle, test_inputs)
H_pred = SHA256("CI:INFERENCE:v1" ∥ count ∥ serialize(predictions))

Where:
    predictions[i] = FORWARD_PASS(bundle.weights, test_inputs[i])
    All computations in Q16.16 fixed-point
```

**Commitment:**
```
H_inference = H_pred
```

**Traceability:** certifiable-inference, CI-MATH-001 §7

---

### §3.6 Monitor Stage Commitment

**Input:** Bundle, predictions, policy from Stages 3-4

**Output:** Final ledger digest

```
L_n = LEDGER_FINAL(bundle, predictions, policy)

Where:
    L_0 = SHA256("CM:LEDGER:GENESIS:v1" ∥ R ∥ H_policy)
    L_t = SHA256("CM:LEDGER:v1" ∥ L_{t-1} ∥ e_t)
    L_n = final digest after all events
```

**Commitment:**
```
H_monitor = L_n
```

**Traceability:** certifiable-monitor, CM-MATH-001 §8

---

### §3.7 Verify Stage Commitment

**Input:** All artifacts from Stages 0-5

**Output:** Verification report hash

```
report = VERIFY_ALL(artifacts)
H_verify = SHA256("CV:REPORT:v1" ∥ serialize(report))

Where:
    report.pipeline_valid = all stages valid AND all bindings valid
```

**Commitment:**
```
H_verify = H(report)
```

**Traceability:** certifiable-verify, CV-MATH-001 §10

---

## §4 Golden Reference

### §4.1 Definition

A **golden reference** is a trusted set of stage commitments generated on a reference platform.

```
G = {
    platform: string,
    timestamp: uint64,
    config_hash: uint8[32],
    harness_hash: uint8[32],
    commitments: uint8[STAGE_COUNT][32]
}
```

**Authority Statement:** The choice of which golden reference is canonical is an operational policy decision outside the scope of this document. Typical policies include:
- First successful run on x86_64 becomes golden
- Signed golden from CI/CD pipeline
- Golden approved by certification authority

### §4.2 Golden Reference Hash

```
H_G = SHA256("CH:GOLDEN:v1" ∥ platform ∥ timestamp ∥ config_hash ∥ 
             harness_hash ∥ commitments[0] ∥ ... ∥ commitments[6])
```

### §4.3 Golden File Format

Binary format for efficient loading:

```
Offset  Size    Field
0       4       Magic ("CHGR")
4       4       Version (1)
8       32      Platform (null-padded)
40      8       Timestamp (little-endian uint64)
48      32      Config hash
80      32      Harness hash (H_H)
112     224     Commitments (7 × 32 bytes)
336     32      File hash (H_G)
368     -       EOF

Total: 368 bytes
```

### §4.4 Golden Generation

```
GENERATE_GOLDEN(config, output_path):
    result = HARNESS_RUN(config)
    
    golden.platform = GET_PLATFORM()
    golden.timestamp = GET_TIMESTAMP()
    golden.config_hash = HASH_CONFIG(config)
    
    FOR s = 0 TO STAGE_COUNT - 1:
        golden.commitments[s] = result.stages[s].hash
    
    golden.file_hash = COMPUTE_GOLDEN_HASH(golden)
    
    WRITE_GOLDEN(golden, output_path)
```

---

## §5 Bit-Identity Verification

### §5.1 Comparison Algorithm

```
COMPARE_GOLDEN(result, golden):
    mismatches = []
    
    FOR s = 0 TO STAGE_COUNT - 1:
        IF result.stages[s].hash ≠ golden.commitments[s]:
            mismatches.append({
                stage: s,
                expected: golden.commitments[s],
                actual: result.stages[s].hash
            })
    
    RETURN {
        bit_identical: (|mismatches| == 0),
        mismatches: mismatches
    }
```

### §5.2 Cross-Platform Comparison

```
COMPARE_PLATFORMS(results[]):
    reference = results[0]
    all_identical = true
    
    FOR i = 1 TO |results| - 1:
        comparison = COMPARE_GOLDEN(results[i], reference)
        IF NOT comparison.bit_identical:
            all_identical = false
            REPORT_DIVERGENCE(reference.platform, results[i].platform, 
                             comparison.mismatches)
    
    RETURN all_identical
```

### §5.3 Divergence Analysis

When bit-identity fails:

```
ANALYZE_DIVERGENCE(expected, actual, stage):
    /* Find first differing byte */
    FOR i = 0 TO 31:
        IF expected[i] ≠ actual[i]:
            first_diff = i
            BREAK
    
    RETURN {
        stage: stage,
        first_diff_byte: first_diff,
        expected_hex: HEX(expected),
        actual_hex: HEX(actual)
    }
```

---

## §6 Harness Configuration

### §6.1 Configuration Structure

```
harness_config_t = {
    data_path: string,          /* Path to test dataset */
    policy_path: string,        /* Path to COE policy JSON */
    golden_path: string | NULL, /* Path to golden reference */
    output_path: string | NULL, /* Path for JSON report */
    
    num_samples: uint32,        /* Samples to process (0 = all) */
    batch_size: uint32,         /* Batch size for training/inference */
    epochs: uint32,             /* Training epochs */
    
    verbose: bool,              /* Enable verbose output */
    generate_golden: bool       /* Generate golden reference */
}
```

### §6.2 Configuration Hash

For reproducibility, the configuration is hashed:

```
H_config = SHA256("CH:CONFIG:v1" ∥ 
                  data_path ∥ policy_path ∥
                  num_samples ∥ batch_size ∥ epochs)
```

### §6.3 Default Configuration

```
DEFAULT_CONFIG = {
    num_samples: 1000,
    batch_size: 32,
    epochs: 10,
    verbose: false,
    generate_golden: false
}
```

---

## §7 Result Structure

### §7.1 Stage Result

```
harness_stage_result_t = {
    stage: harness_stage_t,
    result: ct_result_t,
    hash: uint8[32],
    tests_run: uint32,
    tests_passed: uint32,
    duration_us: uint64
}
```

### §7.2 Pipeline Result

```
harness_result_t = {
    stages: harness_stage_result_t[STAGE_COUNT],
    stages_completed: uint32,
    all_passed: bool,
    bit_identical: bool,
    platform: string[32]
}
```

### §7.3 Result Validity

```
result.all_passed = ∀s: result.stages[s].result == CT_OK
result.bit_identical = COMPARE_GOLDEN(result, golden).bit_identical
```

---

## §8 Report Format

### §8.1 JSON Report Structure

```json
{
    "version": "1.0.0",
    "platform": "x86_64",
    "timestamp": 1737302400,
    "config_hash": "abcd1234...",
    "stages": [
        {
            "stage": 0,
            "name": "data",
            "result": "OK",
            "hash": "1234abcd...",
            "tests_run": 142,
            "tests_passed": 142,
            "duration_us": 12345
        }
    ],
    "all_passed": true,
    "bit_identical": true,
    "golden_comparison": {
        "golden_platform": "x86_64",
        "golden_timestamp": 1737200000,
        "mismatches": []
    },
    "report_hash": "5678efgh..."
}
```

### §8.2 Report Hash

```
H_report = SHA256("CH:REPORT:v1" ∥ version ∥ platform ∥ timestamp ∥
                  config_hash ∥ stages[0..6] ∥ all_passed ∥ bit_identical)
```

---

## §9 Execution Model

### §9.1 Sequential Execution

```
HARNESS_RUN(config):
    result = INIT_RESULT()
    context = INIT_CONTEXT()
    
    /* Stage 0: Data */
    context.data = STAGE_DATA_RUN(config)
    result.stages[0] = CAPTURE_RESULT(context.data)
    
    /* Stage 1: Training */
    context.training = STAGE_TRAINING_RUN(context.data, config)
    result.stages[1] = CAPTURE_RESULT(context.training)
    
    /* Stage 2: Quant */
    context.quant = STAGE_QUANT_RUN(context.training, config)
    result.stages[2] = CAPTURE_RESULT(context.quant)
    
    /* Stage 3: Deploy */
    context.deploy = STAGE_DEPLOY_RUN(context.quant, config)
    result.stages[3] = CAPTURE_RESULT(context.deploy)
    
    /* Stage 4: Inference */
    context.inference = STAGE_INFERENCE_RUN(context.deploy, config)
    result.stages[4] = CAPTURE_RESULT(context.inference)
    
    /* Stage 5: Monitor */
    context.monitor = STAGE_MONITOR_RUN(context.deploy, context.inference, config)
    result.stages[5] = CAPTURE_RESULT(context.monitor)
    
    /* Stage 6: Verify */
    context.verify = STAGE_VERIFY_RUN(context, config)
    result.stages[6] = CAPTURE_RESULT(context.verify)
    
    /* Golden comparison */
    IF config.golden_path ≠ NULL:
        golden = LOAD_GOLDEN(config.golden_path)
        comparison = COMPARE_GOLDEN(result, golden)
        result.bit_identical = comparison.bit_identical
    
    RETURN result
```

### §9.2 Stage Isolation

Each stage:
1. Receives only its declared inputs
2. Produces only its declared outputs
3. Has no side effects on other stages
4. Is independently testable

### §9.3 Timing Measurement

```
MEASURE_STAGE(stage_fn, inputs):
    start = GET_TIME_US()
    output = stage_fn(inputs)
    end = GET_TIME_US()
    duration = end - start
    RETURN (output, duration)
```

---

## §10 Platform Detection

### §10.1 Platform Strings

| Architecture | String |
|--------------|--------|
| x86-64 | `"x86_64"` |
| ARM64 | `"aarch64"` |
| RISC-V 64 | `"riscv64"` |
| Unknown | `"unknown"` |

### §10.2 Detection Algorithm

```
GET_PLATFORM():
    #if defined(__x86_64__) || defined(_M_X64)
        RETURN "x86_64"
    #elif defined(__aarch64__) || defined(_M_ARM64)
        RETURN "aarch64"
    #elif defined(__riscv) && (__riscv_xlen == 64)
        RETURN "riscv64"
    #else
        RETURN "unknown"
    #endif
```

---

## §11 Error Handling

### §11.1 Stage Failure

If any stage fails:
1. Record the failure in `result.stages[s].result`
2. Continue to next stage **only if possible** (see below)
3. Set `result.all_passed = false`

**Stage Execution Precondition:**
```
A stage MAY execute only if all of its declared inputs are available
and valid. Missing or invalid inputs SHALL cause the stage to be SKIPPED.
```

Skipped stages have:
- `result = CH_ERR_SKIPPED`
- `hash = zeros[32]`
- `duration_us = 0`

### §11.2 Fatal Errors

Fatal errors that abort execution:
- NULL configuration
- Missing required input file
- Memory allocation failure (should not occur in DVM)

### §11.3 Error Codes

```
CH_OK = 0               /* Success */
CH_ERR_NULL             /* NULL pointer */
CH_ERR_CONFIG           /* Invalid configuration */
CH_ERR_IO               /* I/O error */
CH_ERR_STAGE            /* Stage execution failed */
CH_ERR_SKIPPED          /* Stage skipped (missing inputs) */
CH_ERR_GOLDEN           /* Golden reference mismatch */
```

---

## §12 Determinism Requirements

### §12.1 Platform Independence

The harness MUST produce bit-identical results on:
- x86-64 (Intel, AMD)
- ARM64 (Apple Silicon, Cortex-A)
- RISC-V 64 (Semper Victus, SiFive)

### §12.2 Compiler Independence

Results MUST be identical with:
- GCC ≥ 7.0
- Clang ≥ 6.0

With flags: `-std=c99 -ffp-contract=off -fno-fast-math`

### §12.3 Endianness

All serialization uses **little-endian** byte order.

### §12.4 Timing Non-Determinism

Timing measurements (`duration_us`) are NOT required to be deterministic.
Only cryptographic commitments (`hash`) must be bit-identical.

---

## §13 Test Data Specification

### §13.1 MNIST Subset Format

```
CSV Format:
    label,pixel_0,pixel_1,...,pixel_783
    5,0,0,0,...,0
    0,0,0,0,...,0
    ...

Fields:
    label: uint8 (0-9)
    pixel_i: uint8 (0-255)
```

### §13.2 Normalization

Pixels are normalized to Q16.16:
```
pixel_q16 = (pixel_u8 << 16) / 255
```

Using integer division with RNE rounding.

### §13.3 Test Dataset Size

Default: 1000 samples (100 per digit)
Maximum: 10000 samples

---

## §14 Domain Separation Tags

| Tag | Value | Used For |
|-----|-------|----------|
| `CH_TAG_GOLDEN` | `"CH:GOLDEN:v1"` | Golden reference hash |
| `CH_TAG_CONFIG` | `"CH:CONFIG:v1"` | Configuration hash |
| `CH_TAG_REPORT` | `"CH:REPORT:v1"` | Report hash |
| `CH_TAG_STAGE` | `"CH:STAGE:v1"` | Stage commitment wrapper |

---

## §15 Computational Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Golden generation | O(pipeline) | One full pipeline run |
| Golden comparison | O(STAGE_COUNT) | 7 hash comparisons |
| Report generation | O(STAGE_COUNT) | Linear in stages |
| Cross-platform compare | O(n × STAGE_COUNT) | n = number of platforms |

---

## Appendix A: Implementation Checklist

### A.1 Required Functions

```
□ harness_run()
□ harness_run_stage()
□ harness_compare_golden()
□ harness_save_golden()
□ harness_load_golden()
□ harness_write_report()
□ harness_get_platform()
□ harness_config_default()
□ harness_result_init()
```

### A.2 Stage Wrappers

```
□ stage_data_run()
□ stage_training_run()
□ stage_quant_run()
□ stage_deploy_run()
□ stage_inference_run()
□ stage_monitor_run()
□ stage_verify_run()
```

### A.3 Test Suites

```
□ test_harness.c         — Main orchestration
□ test_golden.c          — Golden reference I/O
□ test_stages.c          — Individual stage wrappers
□ test_report.c          — Report generation
□ test_bit_identity.c    — Cross-platform verification
```

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2026-01-19 | William Murray | Initial draft |

---

**Document Status:** Draft — Pending implementation.

**Traceability:** All code in `certifiable-harness/src/` SHALL reference this document via `@traceability CH-MATH-001 §N.N` comments.

---

*The harness is the final proof: same inputs → same outputs, every platform, every time.*

---

*Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.*
