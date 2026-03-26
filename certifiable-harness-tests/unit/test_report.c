/**
 * @file test_report.c
 * @brief Tests for report generation
 * @traceability CH-STRUCT-001
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_report.h"
#include "ch_harness.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL: %s (line %d)\n", #cond, __LINE__); \
        return 0; \
    } \
} while(0)

#define RUN_TEST(fn) do { \
    printf("  %s... ", #fn); \
    if (fn()) { printf("PASS\n"); passed++; } \
    else { printf("FAIL\n"); } \
    total++; \
} while(0)

static int test_report_write_json(void)
{
    ch_config_t config = ch_config_default();
    ch_result_t_full result;
    ch_fault_flags_t faults = {0};

    ch_harness_run(&config, &result, &faults);

    const char *path = "/tmp/test_report.json";
    ch_result_t rc = ch_report_write_json(&result, NULL, path, &faults);

    ASSERT(rc == CH_OK);
    ASSERT(access(path, F_OK) == 0);

    /* Cleanup */
    unlink(path);

    return 1;
}

static int test_report_null_safety(void)
{
    ch_result_t_full result;
    ch_fault_flags_t faults = {0};

    ASSERT(ch_report_write_json(NULL, NULL, "test.json", &faults) == CH_ERR_NULL);
    ASSERT(ch_report_write_json(&result, NULL, NULL, &faults) == CH_ERR_NULL);

    return 1;
}

int main(void)
{
    int passed = 0, total = 0;

    printf("\n=== test_report ===\n\n");

    printf("JSON Output:\n");
    RUN_TEST(test_report_write_json);

    printf("\nNull Safety:\n");
    RUN_TEST(test_report_null_safety);

    printf("\n%d/%d tests passed\n\n", passed, total);
    return (passed == total) ? 0 : 1;
}
