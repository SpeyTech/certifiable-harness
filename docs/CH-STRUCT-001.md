# CH-STRUCT-001: Certifiable Harness Data Structure Specification

**Document ID:** CH-STRUCT-001  
**Version:** 1.0.0  
**Status:** Release  
**Date:** 2026-01-19  
**Author:** William Murray  
**Traceability:** CH-MATH-001

---

## 1. Purpose

This document specifies all data structures for certifiable-harness. Every structure traces directly to CH-MATH-001 mathematical definitions.

---

## 2. Constants

### 2.1 Core Constants

| Constant | Value | Description | Reference |
|----------|-------|-------------|-----------|
| `CH_HASH_SIZE` | 32 | SHA-256 digest size in bytes | CH-MATH-001 §1.2 |
| `CH_PLATFORM_SIZE` | 32 | Maximum platform string length | CH-MATH-001 §10.2 |
| `CH_MAX_SAMPLES` | 10000 | Maximum test samples | CH-MATH-001 §13.1 |
| `CH_STAGE_COUNT` | 7 | Number of pipeline stages | CH-MATH-001 §2.1 |

### 2.2 Golden Reference Constants

| Constant | Value | Description | Reference |
|----------|-------|-------------|-----------|
| `CH_GOLDEN_MAGIC` | 0x52474843 | "CHGR" in little-endian | CH-MATH-001 §4.1 |
| `CH_GOLDEN_VERSION` | 1 | Golden format version | CH-MATH-001 §4.1 |
| `CH_GOLDEN_SIZE` | 368 | Total golden file size | CH-MATH-001 §4.1 |

### 2.3 Version Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `CH_VERSION_MAJOR` | 1 | Major version |
| `CH_VERSION_MINOR` | 0 | Minor version |
| `CH_VERSION_PATCH` | 0 | Patch version |
| `CH_VERSION_STRING` | "1.0.0" | Version string |

---

## 3. Enumerations

### 3.1 Result Codes (`ch_result_t`)

**Traceability:** CH-MATH-001 §11.3

```c
typedef enum {
    CH_OK = 0,              /* Success */
    CH_ERR_NULL,            /* NULL pointer argument */
    CH_ERR_CONFIG,          /* Invalid configuration */
    CH_ERR_IO,              /* I/O error */
    CH_ERR_STAGE,           /* Stage execution failed */
    CH_ERR_SKIPPED,         /* Stage skipped (missing inputs) */
    CH_ERR_GOLDEN,          /* Golden reference mismatch */
    CH_ERR_PARSE,           /* Parse error */
    CH_ERR_OVERFLOW         /* Buffer overflow */
} ch_result_t;
```

| Code | Value | Description | Recovery |
|------|-------|-------------|----------|
| `CH_OK` | 0 | Operation succeeded | Continue |
| `CH_ERR_NULL` | 1 | NULL pointer passed | Abort function |
| `CH_ERR_CONFIG` | 2 | Invalid configuration | Check config |
| `CH_ERR_IO` | 3 | File I/O failed | Check paths |
| `CH_ERR_STAGE` | 4 | Stage execution failed | Check stage output |
| `CH_ERR_SKIPPED` | 5 | Stage skipped due to missing inputs | Check dependencies |
| `CH_ERR_GOLDEN` | 6 | Golden comparison failed | Investigate divergence |
| `CH_ERR_PARSE` | 7 | Parse error in input | Check input format |
| `CH_ERR_OVERFLOW` | 8 | Buffer overflow detected | Increase buffer |

### 3.2 Stage Identifiers (`ch_stage_t`)

**Traceability:** CH-MATH-001 §2.1

```c
typedef enum {
    CH_STAGE_DATA      = 0,     /* certifiable-data */
    CH_STAGE_TRAINING  = 1,     /* certifiable-training */
    CH_STAGE_QUANT     = 2,     /* certifiable-quant */
    CH_STAGE_DEPLOY    = 3,     /* certifiable-deploy */
    CH_STAGE_INFERENCE = 4,     /* certifiable-inference */
    CH_STAGE_MONITOR   = 5,     /* certifiable-monitor */
    CH_STAGE_VERIFY    = 6      /* certifiable-verify */
} ch_stage_t;
```

| Stage | Value | Project | Commitment |
|-------|-------|---------|------------|
| `CH_STAGE_DATA` | 0 | certifiable-data | Merkle root M_data |
| `CH_STAGE_TRAINING` | 1 | certifiable-training | Chain hash H_train |
| `CH_STAGE_QUANT` | 2 | certifiable-quant | Certificate H_cert |
| `CH_STAGE_DEPLOY` | 3 | certifiable-deploy | Attestation root R |
| `CH_STAGE_INFERENCE` | 4 | certifiable-inference | Predictions H_pred |
| `CH_STAGE_MONITOR` | 5 | certifiable-monitor | Ledger digest L_n |
| `CH_STAGE_VERIFY` | 6 | certifiable-verify | Report hash H_rpt |

---

## 4. Core Structures

### 4.1 Fault Flags (`ch_fault_flags_t`)

**Purpose:** Accumulates fault conditions during execution.

**Traceability:** DVM specification (shared across certifiable-*)

```c
typedef struct {
    uint32_t overflow    : 1;   /* Arithmetic overflow */
    uint32_t underflow   : 1;   /* Arithmetic underflow */
    uint32_t div_zero    : 1;   /* Division by zero */
    uint32_t domain      : 1;   /* Domain error */
    uint32_t precision   : 1;   /* Precision loss */
    uint32_t _reserved   : 27;  /* Reserved for future use */
} ch_fault_flags_t;
```

| Field | Bit | Description |
|-------|-----|-------------|
| `overflow` | 0 | Result exceeded maximum representable value |
| `underflow` | 1 | Result below minimum representable value |
| `div_zero` | 2 | Division by zero attempted |
| `domain` | 3 | Input outside valid domain |
| `precision` | 4 | Precision lost during operation |
| `_reserved` | 5-31 | Reserved, must be zero |

**Size:** 4 bytes

### 4.2 Stage Result (`ch_stage_result_t`)

**Purpose:** Captures the result of a single stage execution.

**Traceability:** CH-MATH-001 §7.1

```c
typedef struct {
    ch_stage_t  stage;                      /* Stage identifier */
    ch_result_t result;                     /* Execution result */
    uint8_t     hash[CH_HASH_SIZE];         /* Stage commitment (32 bytes) */
    uint32_t    tests_run;                  /* Number of tests executed */
    uint32_t    tests_passed;               /* Number of tests passed */
    uint64_t    duration_us;                /* Execution time in microseconds */
} ch_stage_result_t;
```

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `stage` | 0 | 4 | Stage identifier |
| `result` | 4 | 4 | Result code |
| `hash` | 8 | 32 | Cryptographic commitment |
| `tests_run` | 40 | 4 | Tests executed |
| `tests_passed` | 44 | 4 | Tests passed |
| `duration_us` | 48 | 8 | Duration in microseconds |

**Size:** 56 bytes

**Invariants:**
- `tests_passed <= tests_run`
- If `result == CH_OK`, then `hash` contains valid commitment
- If `result == CH_ERR_SKIPPED`, then `hash` is all zeros

### 4.3 Pipeline Result (`ch_result_t_full`)

**Purpose:** Captures the complete pipeline execution result.

**Traceability:** CH-MATH-001 §7.2

```c
typedef struct {
    ch_stage_result_t stages[CH_STAGE_COUNT];   /* Results for all 7 stages */
    uint32_t          stages_completed;          /* Number of stages completed */
    bool              all_passed;                /* True if all stages passed */
    bool              bit_identical;             /* True if matches golden */
    char              platform[CH_PLATFORM_SIZE];/* Platform identifier */
    uint8_t           harness_hash[CH_HASH_SIZE];/* Harness binary hash H_H */
} ch_result_t_full;
```

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `stages` | 0 | 392 | Array of 7 stage results |
| `stages_completed` | 392 | 4 | Count of completed stages |
| `all_passed` | 396 | 1 | All stages passed flag |
| `bit_identical` | 397 | 1 | Golden comparison result |
| `platform` | 398 | 32 | Platform string |
| `harness_hash` | 430 | 32 | Harness self-hash |

**Size:** 462 bytes (padded to 464)

**Invariants:**
- `stages_completed <= CH_STAGE_COUNT`
- `all_passed == true` iff all stages have `result == CH_OK`
- `bit_identical` only meaningful when golden reference provided

---

## 5. Golden Reference Structure

### 5.1 Golden Reference (`ch_golden_t`)

**Purpose:** Binary format for cross-platform verification.

**Traceability:** CH-MATH-001 §4.1

```c
typedef struct {
    uint32_t magic;                                  /* "CHGR" = 0x52474843 */
    uint32_t version;                                /* Format version = 1 */
    char     platform[CH_PLATFORM_SIZE];             /* Source platform */
    uint64_t timestamp;                              /* Unix timestamp */
    uint8_t  config_hash[CH_HASH_SIZE];              /* Configuration hash */
    uint8_t  harness_hash[CH_HASH_SIZE];             /* Harness binary hash */
    uint8_t  commitments[CH_STAGE_COUNT][CH_HASH_SIZE]; /* Stage commitments */
    uint8_t  file_hash[CH_HASH_SIZE];                /* Self-hash of file */
} ch_golden_t;
```

| Field | Offset | Size | Description |
|-------|--------|------|-------------|
| `magic` | 0x00 | 4 | Magic number "CHGR" |
| `version` | 0x04 | 4 | Format version |
| `platform` | 0x08 | 32 | Platform identifier |
| `timestamp` | 0x28 | 8 | Generation timestamp |
| `config_hash` | 0x30 | 32 | H(config) |
| `harness_hash` | 0x50 | 32 | H(harness binary) |
| `commitments` | 0x70 | 224 | 7 × 32-byte hashes |
| `file_hash` | 0x150 | 32 | H(bytes 0x00-0x14F) |

**Total Size:** 368 bytes

**Binary Layout:**

```
Offset  Size  Field
------  ----  -----
0x000   4     magic ("CHGR" = 0x43 0x48 0x47 0x52)
0x004   4     version (0x01 0x00 0x00 0x00)
0x008   32    platform (null-terminated string)
0x028   8     timestamp (little-endian uint64)
0x030   32    config_hash
0x050   32    harness_hash
0x070   32    commitments[0] (data)
0x090   32    commitments[1] (training)
0x0B0   32    commitments[2] (quant)
0x0D0   32    commitments[3] (deploy)
0x0F0   32    commitments[4] (inference)
0x110   32    commitments[5] (monitor)
0x130   32    commitments[6] (verify)
0x150   32    file_hash
------
0x170   (368 bytes total)
```

**Invariants:**
- `magic == 0x52474843`
- `version == 1`
- `file_hash == SHA256(bytes[0:0x150])`

---

## 6. Configuration Structure

### 6.1 Harness Configuration (`ch_config_t`)

**Purpose:** Runtime configuration for harness execution.

**Traceability:** CH-MATH-001 §6.1

```c
typedef struct {
    const char *data_path;          /* Path to test dataset */
    const char *policy_path;        /* Path to COE policy JSON */
    const char *golden_path;        /* Path to golden reference */
    const char *output_path;        /* Path for JSON report */
    
    uint32_t    num_samples;        /* Samples to process (0 = all) */
    uint32_t    batch_size;         /* Batch size for training/inference */
    uint32_t    epochs;             /* Training epochs */
    
    bool        verbose;            /* Enable verbose output */
    bool        generate_golden;    /* Generate golden reference */
} ch_config_t;
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `data_path` | string | NULL | Path to CSV test data |
| `policy_path` | string | NULL | Path to COE policy |
| `golden_path` | string | NULL | Path to golden (NULL = skip compare) |
| `output_path` | string | NULL | Path for JSON output |
| `num_samples` | uint32 | 1000 | Number of test samples |
| `batch_size` | uint32 | 32 | Batch size |
| `epochs` | uint32 | 10 | Training epochs |
| `verbose` | bool | false | Verbose output |
| `generate_golden` | bool | false | Generate golden file |

**Default Configuration:**

```c
ch_config_t ch_config_default(void) {
    ch_config_t config = {0};
    config.num_samples = 1000;
    config.batch_size = 32;
    config.epochs = 10;
    config.verbose = false;
    config.generate_golden = false;
    return config;
}
```

---

## 7. Context Structure

### 7.1 Pipeline Context (`ch_context_t`)

**Purpose:** Carries state between pipeline stages.

**Traceability:** CH-MATH-001 §9.2

```c
typedef struct {
    /* Stage 0: Data */
    uint8_t data_merkle_root[CH_HASH_SIZE];
    void   *batches;
    size_t  batch_count;
    
    /* Stage 1: Training */
    uint8_t training_hash[CH_HASH_SIZE];
    void   *weights_fp32;
    size_t  weights_size;
    
    /* Stage 2: Quant */
    uint8_t quant_hash[CH_HASH_SIZE];
    void   *weights_q16;
    void   *certificate;
    
    /* Stage 3: Deploy */
    uint8_t deploy_root[CH_HASH_SIZE];
    void   *bundle;
    
    /* Stage 4: Inference */
    uint8_t inference_hash[CH_HASH_SIZE];
    void   *predictions;
    size_t  prediction_count;
    
    /* Stage 5: Monitor */
    uint8_t monitor_digest[CH_HASH_SIZE];
    void   *ledger;
    
    /* Stage 6: Verify */
    uint8_t verify_hash[CH_HASH_SIZE];
    void   *report;
    
    /* Validity flags */
    bool stage_valid[CH_STAGE_COUNT];
} ch_context_t;
```

**Stage Dependencies:**

```
Stage 0 (data)      → independent
Stage 1 (training)  → requires stage 0
Stage 2 (quant)     → requires stage 1
Stage 3 (deploy)    → requires stage 2
Stage 4 (inference) → requires stage 3
Stage 5 (monitor)   → requires stages 3, 4
Stage 6 (verify)    → requires stages 0-5
```

**Invariant:** `stage_valid[s]` is true iff stage `s` completed successfully.

---

## 8. Domain Separation Tags

**Traceability:** CH-MATH-001 §14

```c
#define CH_TAG_GOLDEN   "CH:GOLDEN:v1"
#define CH_TAG_CONFIG   "CH:CONFIG:v1"
#define CH_TAG_REPORT   "CH:REPORT:v1"
#define CH_TAG_STAGE    "CH:STAGE:v1"
```

| Tag | Usage |
|-----|-------|
| `CH:GOLDEN:v1` | Golden file hash preimage |
| `CH:CONFIG:v1` | Configuration hash preimage |
| `CH:REPORT:v1` | Report hash preimage |
| `CH:STAGE:v1` | Stage commitment preimage |

---

## 9. Helper Functions

### 9.1 Fault Flag Helpers

```c
static inline bool ch_has_fault(const ch_fault_flags_t *f) {
    return f->overflow || f->underflow || f->div_zero || 
           f->domain || f->precision;
}

static inline void ch_clear_faults(ch_fault_flags_t *f) {
    f->overflow = 0;
    f->underflow = 0;
    f->div_zero = 0;
    f->domain = 0;
    f->precision = 0;
}
```

### 9.2 Stage Name Helper

```c
static inline const char *ch_stage_name(ch_stage_t stage) {
    static const char *names[] = {
        "data", "training", "quant", "deploy",
        "inference", "monitor", "verify"
    };
    return (stage < CH_STAGE_COUNT) ? names[stage] : "unknown";
}
```

### 9.3 Result String Helper

```c
static inline const char *ch_result_string(ch_result_t r) {
    static const char *strings[] = {
        "OK", "NULL", "CONFIG", "IO", "STAGE", 
        "SKIPPED", "GOLDEN", "PARSE", "OVERFLOW"
    };
    return (r <= CH_ERR_OVERFLOW) ? strings[r] : "UNKNOWN";
}
```

---

## 10. Memory Layout Summary

| Structure | Size (bytes) | Alignment |
|-----------|--------------|-----------|
| `ch_fault_flags_t` | 4 | 4 |
| `ch_stage_result_t` | 56 | 8 |
| `ch_result_t_full` | 464 | 8 |
| `ch_golden_t` | 368 | 8 |
| `ch_config_t` | ~72 | 8 |
| `ch_context_t` | ~512 | 8 |

All structures use little-endian byte order for serialization.

---

## 11. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2026-01-19 | William Murray | Initial release |

---

*Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.*
