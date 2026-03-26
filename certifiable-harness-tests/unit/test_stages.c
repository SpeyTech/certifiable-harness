/**
 * @file test_stages.c
 * @brief Tests for stage wrappers
 * @traceability CH-STRUCT-001
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_stages.h"
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

static int test_context_init(void)
{
    ch_context_t ctx;
    ch_context_init(&ctx);

    for (int s = 0; s < CH_STAGE_COUNT; s++) {
        ASSERT(ctx.stage_valid[s] == false);
    }

    return 1;
}

static int test_stage_data(void)
{
    ch_config_t config = ch_config_default();
    ch_context_t ctx;
    ch_stage_result_t result;
    ch_fault_flags_t faults = {0};

    ch_context_init(&ctx);

    ch_result_t rc = ch_stage_data(&config, &ctx, &result, &faults);

    ASSERT(rc == CH_OK);
    ASSERT(result.stage == CH_STAGE_DATA);
    ASSERT(result.result == CH_OK);
    ASSERT(ctx.stage_valid[CH_STAGE_DATA] == true);

    return 1;
}

static int test_all_stages_run(void)
{
    ch_config_t config = ch_config_default();
    ch_context_t ctx;
    ch_stage_result_t result;
    ch_fault_flags_t faults = {0};

    ch_context_init(&ctx);

    /* Run all stages in sequence */
    ASSERT(ch_stage_data(&config, &ctx, &result, &faults) == CH_OK);
    ASSERT(ch_stage_training(&config, &ctx, &result, &faults) == CH_OK);
    ASSERT(ch_stage_quant(&config, &ctx, &result, &faults) == CH_OK);
    ASSERT(ch_stage_deploy(&config, &ctx, &result, &faults) == CH_OK);
    ASSERT(ch_stage_inference(&config, &ctx, &result, &faults) == CH_OK);
    ASSERT(ch_stage_monitor(&config, &ctx, &result, &faults) == CH_OK);
    ASSERT(ch_stage_verify(&config, &ctx, &result, &faults) == CH_OK);

    ch_context_free(&ctx);

    return 1;
}

static int test_stage_null_safety(void)
{
    ch_config_t config = ch_config_default();
    ch_context_t ctx;
    ch_stage_result_t result;
    ch_fault_flags_t faults = {0};

    ch_context_init(&ctx);

    ASSERT(ch_stage_data(NULL, &ctx, &result, &faults) == CH_ERR_NULL);
    ASSERT(ch_stage_data(&config, NULL, &result, &faults) == CH_ERR_NULL);
    ASSERT(ch_stage_data(&config, &ctx, NULL, &faults) == CH_ERR_NULL);
    ASSERT(ch_stage_data(&config, &ctx, &result, NULL) == CH_ERR_NULL);

    return 1;
}

int main(void)
{
    int passed = 0, total = 0;

    printf("\n=== test_stages ===\n\n");

    printf("Context:\n");
    RUN_TEST(test_context_init);

    printf("\nStages:\n");
    RUN_TEST(test_stage_data);
    RUN_TEST(test_all_stages_run);

    printf("\nNull Safety:\n");
    RUN_TEST(test_stage_null_safety);

    printf("\n%d/%d tests passed\n\n", passed, total);
    return (passed == total) ? 0 : 1;
}
