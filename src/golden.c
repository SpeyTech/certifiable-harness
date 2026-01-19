/**
 * @file golden.c
 * @brief Golden reference I/O
 * @traceability CH-MATH-001 §4, §5
 *
 * Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_golden.h"
#include "ch_harness.h"
#include "ch_hash.h"
#include <stdio.h>
#include <string.h>

/* @traceability CH-MATH-001 §4.3 */
ch_result_t ch_golden_load(const char *path,
                            ch_golden_t *golden,
                            ch_fault_flags_t *faults)
{
    if (!path || !golden || !faults) {
        return CH_ERR_NULL;
    }
    
    FILE *f = fopen(path, "rb");
    if (!f) {
        return CH_ERR_IO;
    }
    
    size_t n = fread(golden, 1, sizeof(*golden), f);
    fclose(f);
    
    if (n != sizeof(*golden)) {
        return CH_ERR_IO;
    }
    
    /* Validate magic */
    if (golden->magic != CH_GOLDEN_MAGIC) {
        return CH_ERR_PARSE;
    }
    
    /* Validate version */
    if (golden->version != CH_GOLDEN_VERSION) {
        return CH_ERR_PARSE;
    }
    
    /* Verify file hash */
    uint8_t computed_hash[CH_HASH_SIZE];
    ch_result_t rc = ch_golden_compute_hash(golden, computed_hash, faults);
    if (rc != CH_OK) {
        return rc;
    }
    
    if (!ch_hash_equal(computed_hash, golden->file_hash)) {
        return CH_ERR_GOLDEN;
    }
    
    return CH_OK;
}

/* @traceability CH-MATH-001 §4.4 */
ch_result_t ch_golden_save(const ch_golden_t *golden,
                            const char *path,
                            ch_fault_flags_t *faults)
{
    if (!golden || !path || !faults) {
        return CH_ERR_NULL;
    }
    
    FILE *f = fopen(path, "wb");
    if (!f) {
        return CH_ERR_IO;
    }
    
    size_t n = fwrite(golden, 1, sizeof(*golden), f);
    fclose(f);
    
    if (n != sizeof(*golden)) {
        return CH_ERR_IO;
    }
    
    return CH_OK;
}

/* @traceability CH-MATH-001 §4.4 */
ch_result_t ch_golden_generate(const ch_result_t_full *result,
                                const ch_config_t *config,
                                ch_golden_t *golden,
                                ch_fault_flags_t *faults)
{
    if (!result || !config || !golden || !faults) {
        return CH_ERR_NULL;
    }
    
    memset(golden, 0, sizeof(*golden));
    
    golden->magic = CH_GOLDEN_MAGIC;
    golden->version = CH_GOLDEN_VERSION;
    strncpy(golden->platform, result->platform, CH_PLATFORM_SIZE - 1);
    golden->timestamp = ch_get_timestamp();
    
    /* TODO: Compute config hash */
    ch_hash_zero(golden->config_hash);
    
    /* Copy harness hash */
    ch_hash_copy(golden->harness_hash, result->harness_hash);
    
    /* Copy stage commitments */
    for (int s = 0; s < CH_STAGE_COUNT; s++) {
        ch_hash_copy(golden->commitments[s], result->stages[s].hash);
    }
    
    /* Compute file hash */
    ch_result_t rc = ch_golden_compute_hash(golden, golden->file_hash, faults);
    
    return rc;
}

/* @traceability CH-MATH-001 §5.1 */
ch_result_t ch_golden_compare(const ch_result_t_full *result,
                               const ch_golden_t *golden,
                               bool *bit_identical,
                               int *first_mismatch,
                               ch_fault_flags_t *faults)
{
    if (!result || !golden || !bit_identical || !first_mismatch || !faults) {
        return CH_ERR_NULL;
    }
    
    *bit_identical = true;
    *first_mismatch = -1;
    
    for (int s = 0; s < CH_STAGE_COUNT; s++) {
        if (!ch_hash_equal(result->stages[s].hash, golden->commitments[s])) {
            *bit_identical = false;
            if (*first_mismatch < 0) {
                *first_mismatch = s;
            }
        }
    }
    
    return CH_OK;
}

/* @traceability CH-MATH-001 §4.2 */
ch_result_t ch_golden_compute_hash(const ch_golden_t *golden,
                                    uint8_t hash_out[CH_HASH_SIZE],
                                    ch_fault_flags_t *faults)
{
    if (!golden || !hash_out || !faults) {
        return CH_ERR_NULL;
    }
    
    /* Hash everything except file_hash field */
    /* file_hash is at offset 336, so hash bytes 0-335 */
    size_t hash_len = offsetof(ch_golden_t, file_hash);
    
    /* Prepend domain tag */
    /* TODO: Implement proper tagged hashing */
    return ch_sha256((const uint8_t*)golden, hash_len, hash_out, faults);
}
