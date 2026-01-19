/**
 * @file harness.c
 * @brief Main harness orchestration
 * @traceability CH-MATH-001 §9
 *
 * Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_harness.h"
#include "ch_stages.h"
#include "ch_hash.h"
#include <string.h>
#include <time.h>

/* @traceability CH-MATH-001 §6.3 */
ch_config_t ch_config_default(void)
{
    ch_config_t config = {0};
    config.num_samples = 1000;
    config.batch_size = 32;
    config.epochs = 10;
    config.verbose = false;
    config.generate_golden = false;
    return config;
}

/* @traceability CH-MATH-001 §9.1 */
ch_result_t ch_harness_run(const ch_config_t *config,
                           ch_result_t_full *result,
                           ch_fault_flags_t *faults)
{
    if (!config || !result || !faults) {
        return CH_ERR_NULL;
    }
    
    /* Initialize result */
    memset(result, 0, sizeof(*result));
    
    /* Get platform and harness hash */
    const char *platform = ch_get_platform();
    strncpy(result->platform, platform, CH_PLATFORM_SIZE - 1);
    ch_get_harness_hash(result->harness_hash, faults);
    
    /* Initialize context */
    ch_context_t ctx;
    ch_context_init(&ctx);
    
    /* Stage function pointers */
    typedef ch_result_t (*stage_fn_t)(const ch_config_t*, ch_context_t*, 
                                       ch_stage_result_t*, ch_fault_flags_t*);
    static const stage_fn_t stage_fns[CH_STAGE_COUNT] = {
        ch_stage_data,
        ch_stage_training,
        ch_stage_quant,
        ch_stage_deploy,
        ch_stage_inference,
        ch_stage_monitor,
        ch_stage_verify
    };
    
    /* Run each stage */
    result->all_passed = true;
    for (int s = 0; s < CH_STAGE_COUNT; s++) {
        ch_stage_result_t *sr = &result->stages[s];
        sr->stage = (ch_stage_t)s;
        
        /* Check if inputs are available */
        bool can_run = true;
        if (s > 0 && !ctx.stage_valid[s - 1]) {
            /* Previous stage failed - check if we can still run */
            /* For now, require all previous stages */
            can_run = false;
        }
        
        if (!can_run) {
            sr->result = CH_ERR_SKIPPED;
            ch_hash_zero(sr->hash);
            sr->duration_us = 0;
            result->all_passed = false;
            continue;
        }
        
        /* Run stage with timing */
        uint64_t start = ch_get_time_us();
        ch_result_t rc = stage_fns[s](config, &ctx, sr, faults);
        uint64_t end = ch_get_time_us();
        
        sr->duration_us = end - start;
        
        if (rc != CH_OK) {
            sr->result = rc;
            result->all_passed = false;
            ctx.stage_valid[s] = false;
        } else {
            ctx.stage_valid[s] = true;
            result->stages_completed++;
        }
    }
    
    /* Cleanup context */
    ch_context_free(&ctx);
    
    return CH_OK;
}

/* @traceability CH-MATH-001 §10.2 */
const char *ch_get_platform(void)
{
#if defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "aarch64";
#elif defined(__riscv) && (__riscv_xlen == 64)
    return "riscv64";
#else
    return "unknown";
#endif
}

uint64_t ch_get_timestamp(void)
{
    return (uint64_t)time(NULL);
}

uint64_t ch_get_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

/* @traceability CH-MATH-001 §1.4 */
ch_result_t ch_get_harness_hash(uint8_t hash_out[CH_HASH_SIZE],
                                 ch_fault_flags_t *faults)
{
    if (!hash_out || !faults) {
        return CH_ERR_NULL;
    }
    
    /* TODO: Read /proc/self/exe and hash it */
    /* For now, return zeros */
    ch_hash_zero(hash_out);
    
    return CH_OK;
}
