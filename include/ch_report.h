/**
 * @file ch_report.h
 * @brief Report generation API
 * @traceability CH-MATH-001 §8
 *
 * Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#ifndef CH_REPORT_H
#define CH_REPORT_H

#include "ch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Report Generation
 *============================================================================*/

/**
 * @brief Write JSON report to file
 * @param result Pipeline result
 * @param golden Golden reference (NULL if not compared)
 * @param path Output path
 * @param faults Fault flag accumulator
 * @return CH_OK on success
 * @traceability CH-MATH-001 §8.1
 */
ch_result_t ch_report_write_json(const ch_result_t_full *result,
                                  const ch_golden_t *golden,
                                  const char *path,
                                  ch_fault_flags_t *faults);

/**
 * @brief Print summary to stdout
 * @param result Pipeline result
 * @param golden Golden reference (NULL if not compared)
 */
void ch_report_print_summary(const ch_result_t_full *result,
                              const ch_golden_t *golden);

#ifdef __cplusplus
}
#endif

#endif /* CH_REPORT_H */
