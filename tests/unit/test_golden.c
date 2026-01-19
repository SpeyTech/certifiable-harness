/**
 * @file test_golden.c
 * @brief Tests for golden reference I/O
 */

#include "ch_golden.h"
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

static int test_golden_generate(void)
{
    ch_config_t config = ch_config_default();
    ch_result_t_full result;
    ch_fault_flags_t faults = {0};
    
    ch_harness_run(&config, &result, &faults);
    
    ch_golden_t golden;
    ch_result_t rc = ch_golden_generate(&result, &config, &golden, &faults);
    
    ASSERT(rc == CH_OK);
    ASSERT(golden.magic == CH_GOLDEN_MAGIC);
    ASSERT(golden.version == CH_GOLDEN_VERSION);
    ASSERT(strlen(golden.platform) > 0);
    
    return 1;
}

static int test_golden_compare_identical(void)
{
    ch_config_t config = ch_config_default();
    ch_result_t_full result;
    ch_fault_flags_t faults = {0};
    
    ch_harness_run(&config, &result, &faults);
    
    ch_golden_t golden;
    ch_golden_generate(&result, &config, &golden, &faults);
    
    bool bit_identical;
    int first_mismatch;
    ch_result_t rc = ch_golden_compare(&result, &golden, &bit_identical, 
                                        &first_mismatch, &faults);
    
    ASSERT(rc == CH_OK);
    ASSERT(bit_identical == true);
    ASSERT(first_mismatch == -1);
    
    return 1;
}

static int test_golden_null_safety(void)
{
    ch_golden_t golden;
    ch_fault_flags_t faults = {0};
    bool bit_identical;
    int first_mismatch;
    ch_result_t_full result;
    
    ASSERT(ch_golden_load(NULL, &golden, &faults) == CH_ERR_NULL);
    ASSERT(ch_golden_save(NULL, "test.bin", &faults) == CH_ERR_NULL);
    ASSERT(ch_golden_compare(NULL, &golden, &bit_identical, &first_mismatch, &faults) == CH_ERR_NULL);
    ASSERT(ch_golden_compare(&result, NULL, &bit_identical, &first_mismatch, &faults) == CH_ERR_NULL);
    
    return 1;
}

int main(void)
{
    int passed = 0, total = 0;
    
    printf("\n=== test_golden ===\n\n");
    
    printf("Generation:\n");
    RUN_TEST(test_golden_generate);
    
    printf("\nComparison:\n");
    RUN_TEST(test_golden_compare_identical);
    
    printf("\nNull Safety:\n");
    RUN_TEST(test_golden_null_safety);
    
    printf("\n%d/%d tests passed\n\n", passed, total);
    return (passed == total) ? 0 : 1;
}
