/**
 * @file ch_types.h
 * @brief Core types for certifiable-harness
 * @traceability CH-MATH-001 2, 7
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#ifndef CH_TYPES_H
#define CH_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Constants
 *============================================================================*/

#define CH_VERSION_MAJOR    1
#define CH_VERSION_MINOR    0
#define CH_VERSION_PATCH    0
#define CH_VERSION_STRING   "1.0.0"

#define CH_HASH_SIZE        32
#define CH_PLATFORM_SIZE    32
#define CH_MAX_SAMPLES      10000
#define CH_STAGE_COUNT      7

/*============================================================================
 * Result Codes
 * @traceability CH-MATH-001 11.3
 *============================================================================*/

typedef enum {
    CH_OK = 0,              /**< Success */
    CH_ERR_NULL,            /**< NULL pointer argument */
    CH_ERR_CONFIG,          /**< Invalid configuration */
    CH_ERR_IO,              /**< I/O error */
    CH_ERR_STAGE,           /**< Stage execution failed */
    CH_ERR_SKIPPED,         /**< Stage skipped (missing inputs) */
    CH_ERR_GOLDEN,          /**< Golden reference mismatch */
    CH_ERR_PARSE,           /**< Parse error */
    CH_ERR_OVERFLOW         /**< Buffer overflow */
} ch_result_t;

/*============================================================================
 * Fault Flags
 *============================================================================*/

typedef struct {
    uint32_t overflow    : 1;
    uint32_t underflow   : 1;
    uint32_t div_zero    : 1;
    uint32_t domain      : 1;
    uint32_t precision   : 1;
    uint32_t _reserved   : 27;
} ch_fault_flags_t;

/*============================================================================
 * Stage Identifiers
 * @traceability CH-MATH-001 2.1
 *============================================================================*/

typedef enum {
    CH_STAGE_DATA      = 0,     /**< certifiable-data */
    CH_STAGE_TRAINING  = 1,     /**< certifiable-training */
    CH_STAGE_QUANT     = 2,     /**< certifiable-quant */
    CH_STAGE_DEPLOY    = 3,     /**< certifiable-deploy */
    CH_STAGE_INFERENCE = 4,     /**< certifiable-inference */
    CH_STAGE_MONITOR   = 5,     /**< certifiable-monitor */
    CH_STAGE_VERIFY    = 6      /**< certifiable-verify */
} ch_stage_t;

/*============================================================================
 * Stage Result
 * @traceability CH-MATH-001 7.1
 *============================================================================*/

typedef struct {
    ch_stage_t  stage;
    ch_result_t result;
    uint8_t     hash[CH_HASH_SIZE];      /**< Stage commitment */
    uint32_t    tests_run;
    uint32_t    tests_passed;
    uint64_t    duration_us;
} ch_stage_result_t;

/*============================================================================
 * Pipeline Result
 * @traceability CH-MATH-001 7.2
 *============================================================================*/

typedef struct {
    ch_stage_result_t stages[CH_STAGE_COUNT];
    uint32_t          stages_completed;
    bool              all_passed;
    bool              bit_identical;
    char              platform[CH_PLATFORM_SIZE];
    uint8_t           harness_hash[CH_HASH_SIZE];   /**< H_H */
} ch_result_t_full;

/*============================================================================
 * Golden Reference
 * @traceability CH-MATH-001 4.1
 *============================================================================*/

#define CH_GOLDEN_MAGIC     0x52474843  /* "CHGR" little-endian */
#define CH_GOLDEN_VERSION   1
#define CH_GOLDEN_SIZE      368

typedef struct {
    uint32_t magic;
    uint32_t version;
    char     platform[CH_PLATFORM_SIZE];
    uint64_t timestamp;
    uint8_t  config_hash[CH_HASH_SIZE];
    uint8_t  harness_hash[CH_HASH_SIZE];
    uint8_t  commitments[CH_STAGE_COUNT][CH_HASH_SIZE];
    uint8_t  file_hash[CH_HASH_SIZE];
} ch_golden_t;

/*============================================================================
 * Configuration
 * @traceability CH-MATH-001 6.1
 *============================================================================*/

typedef struct {
    const char *data_path;          /**< Path to test dataset */
    const char *policy_path;        /**< Path to COE policy JSON */
    const char *golden_path;        /**< Path to golden reference (NULL to skip) */
    const char *output_path;        /**< Path for JSON report (NULL to skip) */

    uint32_t    num_samples;        /**< Samples to process (0 = all) */
    uint32_t    batch_size;         /**< Batch size for training/inference */
    uint32_t    epochs;             /**< Training epochs */

    bool        verbose;            /**< Enable verbose output */
    bool        generate_golden;    /**< Generate golden reference */
} ch_config_t;

/*============================================================================
 * Helper Functions
 *============================================================================*/

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

static inline const char *ch_stage_name(ch_stage_t stage) {
    static const char *names[] = {
        "data", "training", "quant", "deploy",
        "inference", "monitor", "verify"
    };
    return (stage < CH_STAGE_COUNT) ? names[stage] : "unknown";
}

static inline const char *ch_result_string(ch_result_t r) {
    static const char *strings[] = {
        "OK", "NULL", "CONFIG", "IO", "STAGE",
        "SKIPPED", "GOLDEN", "PARSE", "OVERFLOW"
    };
    return (r <= CH_ERR_OVERFLOW) ? strings[r] : "UNKNOWN";
}

#ifdef __cplusplus
}
#endif

#endif /* CH_TYPES_H */
