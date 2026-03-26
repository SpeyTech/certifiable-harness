/**
 * @file test_harness.c
 * @brief Tests for harness core
 * @traceability CH-STRUCT-001
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_harness.h"
#include "ch_hash.h"
#include <stdio.h>
#include <string.h>

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

static int test_config_default(void)
{
    ch_config_t config = ch_config_default();

    ASSERT(config.num_samples == 1000);
    ASSERT(config.batch_size == 32);
    ASSERT(config.epochs == 10);
    ASSERT(config.verbose == false);
    ASSERT(config.generate_golden == false);

    return 1;
}

static int test_harness_run_null(void)
{
    ch_result_t_full result;
    ch_fault_flags_t faults = {0};
    ch_config_t config = ch_config_default();

    ASSERT(ch_harness_run(NULL, &result, &faults) == CH_ERR_NULL);
    ASSERT(ch_harness_run(&config, NULL, &faults) == CH_ERR_NULL);
    ASSERT(ch_harness_run(&config, &result, NULL) == CH_ERR_NULL);

    return 1;
}

static int test_harness_run_basic(void)
{
    ch_config_t config = ch_config_default();
    ch_result_t_full result;
    ch_fault_flags_t faults = {0};

    ch_result_t rc = ch_harness_run(&config, &result, &faults);

    ASSERT(rc == CH_OK);
    ASSERT(result.stages_completed == CH_STAGE_COUNT);
    ASSERT(result.all_passed == true);
    ASSERT(strlen(result.platform) > 0);

    return 1;
}

static int test_get_platform(void)
{
    const char *platform = ch_get_platform();

    ASSERT(platform != NULL);
    ASSERT(strlen(platform) > 0);
    ASSERT(strlen(platform) < CH_PLATFORM_SIZE);

    /* Should be one of known platforms */
    ASSERT(strcmp(platform, "x86_64") == 0 ||
           strcmp(platform, "aarch64") == 0 ||
           strcmp(platform, "riscv64") == 0 ||
           strcmp(platform, "unknown") == 0);

    return 1;
}

int main(void)
{
    int passed = 0, total = 0;

    printf("\n=== test_harness ===\n\n");

    printf("Configuration:\n");
    RUN_TEST(test_config_default);

    printf("\nNull Safety:\n");
    RUN_TEST(test_harness_run_null);

    printf("\nBasic Execution:\n");
    RUN_TEST(test_harness_run_basic);

    printf("\nPlatform:\n");
    RUN_TEST(test_get_platform);

    printf("\n%d/%d tests passed\n\n", passed, total);
    return (passed == total) ? 0 : 1;
}
