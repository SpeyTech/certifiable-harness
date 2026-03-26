/**
 * @file ch_harness.h
 * @brief Main harness API
 * @traceability CH-MATH-001 9
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#ifndef CH_HARNESS_H
#define CH_HARNESS_H

#include "ch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Configuration
 *============================================================================*/

/**
 * @brief Get default configuration
 * @traceability CH-MATH-001 6.3
 */
ch_config_t ch_config_default(void);

/*============================================================================
 * Main API
 *============================================================================*/

/**
 * @brief Run complete pipeline
 * @param config Harness configuration
 * @param result Output result structure
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 9.1
 */
ch_result_t ch_harness_run(const ch_config_t *config,
                           ch_result_t_full *result,
                           ch_fault_flags_t *faults);

/**
 * @brief Run single stage
 * @param stage Stage to run
 * @param config Harness configuration
 * @param context Pipeline context (previous stage outputs)
 * @param result Output stage result
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 9.1
 */
ch_result_t ch_harness_run_stage(ch_stage_t stage,
                                  const ch_config_t *config,
                                  void *context,
                                  ch_stage_result_t *result,
                                  ch_fault_flags_t *faults);

/*============================================================================
 * Platform Detection
 *============================================================================*/

/**
 * @brief Get platform identifier string
 * @return Platform string (static, do not free)
 * @traceability CH-MATH-001 10.2
 */
const char *ch_get_platform(void);

/**
 * @brief Get harness binary hash
 * @param hash_out Output buffer for 32-byte hash
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 1.4
 */
ch_result_t ch_get_harness_hash(uint8_t hash_out[CH_HASH_SIZE],
                                 ch_fault_flags_t *faults);

/**
 * @brief Get current timestamp (Unix seconds)
 */
uint64_t ch_get_timestamp(void);

/**
 * @brief Get current time in microseconds
 */
uint64_t ch_get_time_us(void);

#ifdef __cplusplus
}
#endif

#endif /* CH_HARNESS_H */
