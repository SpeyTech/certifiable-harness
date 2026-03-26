/**
 * @file report.c
 * @brief Report generation
 * @traceability CH-MATH-001 8
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_report.h"
#include "ch_hash.h"
#include <stdio.h>

/* @traceability CH-MATH-001 8.1 */
ch_result_t ch_report_write_json(const ch_result_t_full *result,
                                  const ch_golden_t *golden,
                                  const char *path,
                                  ch_fault_flags_t *faults)
{
    if (!result || !path || !faults) {
        (void)golden; /* Future: include golden comparison */
    return CH_ERR_NULL;
    }

    FILE *f = fopen(path, "w");
    if (!f) {
        return CH_ERR_IO;
    }

    char hash_hex[65];

    fprintf(f, "{\n");
    fprintf(f, "  \"version\": \"%s\",\n", CH_VERSION_STRING);
    fprintf(f, "  \"platform\": \"%s\",\n", result->platform);

    ch_hash_to_hex(result->harness_hash, hash_hex);
    fprintf(f, "  \"harness_hash\": \"%s\",\n", hash_hex);

    fprintf(f, "  \"stages\": [\n");
    for (int s = 0; s < CH_STAGE_COUNT; s++) {
        const ch_stage_result_t *sr = &result->stages[s];
        ch_hash_to_hex(sr->hash, hash_hex);

        fprintf(f, "    {\n");
        fprintf(f, "      \"stage\": %d,\n", s);
        fprintf(f, "      \"name\": \"%s\",\n", ch_stage_name(sr->stage));
        fprintf(f, "      \"result\": \"%s\",\n", ch_result_string(sr->result));
        fprintf(f, "      \"hash\": \"%s\",\n", hash_hex);
        fprintf(f, "      \"duration_us\": %lu\n", (unsigned long)sr->duration_us);
        fprintf(f, "    }%s\n", (s < CH_STAGE_COUNT - 1) ? "," : "");
    }
    fprintf(f, "  ],\n");

    fprintf(f, "  \"stages_completed\": %u,\n", result->stages_completed);
    fprintf(f, "  \"all_passed\": %s,\n", result->all_passed ? "true" : "false");
    fprintf(f, "  \"bit_identical\": %s\n", result->bit_identical ? "true" : "false");

    fprintf(f, "}\n");

    fclose(f);
    return CH_OK;
}

void ch_report_print_summary(const ch_result_t_full *result,
                              const ch_golden_t *golden)
{
    if (!result) return;

    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  Certifiable Harness — Pipeline Summary\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("  Platform: %s\n", result->platform);
    printf("  Stages completed: %u/%d\n", result->stages_completed, CH_STAGE_COUNT);
    printf("\n");

    for (int s = 0; s < CH_STAGE_COUNT; s++) {
        const ch_stage_result_t *sr = &result->stages[s];
        const char *status = (sr->result == CH_OK) ? "✓" :
                             (sr->result == CH_ERR_SKIPPED) ? "⊘" : "✗";
        printf("  [%d] %-12s %s  (%s, %lu µs)\n",
               s, ch_stage_name(sr->stage), status,
               ch_result_string(sr->result),
               (unsigned long)sr->duration_us);
    }

    printf("\n");
    if (result->all_passed) {
        printf("  Status: ALL STAGES PASSED ✓\n");
    } else {
        printf("  Status: PIPELINE FAILED ✗\n");
    }

    if (golden) {
        printf("  Bit-identical: %s\n", result->bit_identical ? "YES ✓" : "NO ✗");
    }

    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}
