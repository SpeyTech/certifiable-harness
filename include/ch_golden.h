/**
 * @file ch_golden.h
 * @brief Golden reference API
 * @traceability CH-MATH-001 §4, §5
 *
 * Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#ifndef CH_GOLDEN_H
#define CH_GOLDEN_H

#include "ch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Golden Reference I/O
 *============================================================================*/

/**
 * @brief Load golden reference from file
 * @param path Path to golden file
 * @param golden Output golden structure
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 §4.3
 */
ch_result_t ch_golden_load(const char *path,
                            ch_golden_t *golden,
                            ch_fault_flags_t *faults);

/**
 * @brief Save golden reference to file
 * @param golden Golden structure to save
 * @param path Output path
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 §4.4
 */
ch_result_t ch_golden_save(const ch_golden_t *golden,
                            const char *path,
                            ch_fault_flags_t *faults);

/**
 * @brief Generate golden from result
 * @param result Pipeline result
 * @param config Configuration used
 * @param golden Output golden structure
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 §4.4
 */
ch_result_t ch_golden_generate(const ch_result_t_full *result,
                                const ch_config_t *config,
                                ch_golden_t *golden,
                                ch_fault_flags_t *faults);

/*============================================================================
 * Comparison
 *============================================================================*/

/**
 * @brief Compare result against golden
 * @param result Pipeline result
 * @param golden Golden reference
 * @param bit_identical Output: true if all hashes match
 * @param first_mismatch Output: first mismatched stage (-1 if none)
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 §5.1
 */
ch_result_t ch_golden_compare(const ch_result_t_full *result,
                               const ch_golden_t *golden,
                               bool *bit_identical,
                               int *first_mismatch,
                               ch_fault_flags_t *faults);

/**
 * @brief Compute golden file hash
 * @param golden Golden structure (file_hash field ignored)
 * @param hash_out Output buffer for 32-byte hash
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 §4.2
 */
ch_result_t ch_golden_compute_hash(const ch_golden_t *golden,
                                    uint8_t hash_out[CH_HASH_SIZE],
                                    ch_fault_flags_t *faults);

#ifdef __cplusplus
}
#endif

#endif /* CH_GOLDEN_H */
