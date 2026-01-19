/**
 * @file ch_stages.h
 * @brief Stage wrapper declarations
 * @traceability CH-MATH-001 §3
 *
 * Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#ifndef CH_STAGES_H
#define CH_STAGES_H

#include "ch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Pipeline Context
 * Holds outputs from each stage for passing to subsequent stages
 *============================================================================*/

typedef struct {
    /* Stage 0: Data */
    uint8_t data_merkle_root[CH_HASH_SIZE];
    void   *batches;
    size_t  batch_count;
    
    /* Stage 1: Training */
    uint8_t training_hash[CH_HASH_SIZE];
    void   *weights_fp32;
    size_t  weights_size;
    
    /* Stage 2: Quant */
    uint8_t quant_hash[CH_HASH_SIZE];
    void   *weights_q16;
    void   *certificate;
    
    /* Stage 3: Deploy */
    uint8_t deploy_root[CH_HASH_SIZE];
    void   *bundle;
    
    /* Stage 4: Inference */
    uint8_t inference_hash[CH_HASH_SIZE];
    void   *predictions;
    size_t  prediction_count;
    
    /* Stage 5: Monitor */
    uint8_t monitor_digest[CH_HASH_SIZE];
    void   *ledger;
    
    /* Stage 6: Verify */
    uint8_t verify_hash[CH_HASH_SIZE];
    void   *report;
    
    /* Validity flags */
    bool stage_valid[CH_STAGE_COUNT];
} ch_context_t;

/*============================================================================
 * Stage Wrappers
 * Each wraps the corresponding certifiable-* library
 *============================================================================*/

/**
 * @brief Run data stage (certifiable-data)
 * @traceability CH-MATH-001 §3.1
 */
ch_result_t ch_stage_data(const ch_config_t *config,
                          ch_context_t *ctx,
                          ch_stage_result_t *result,
                          ch_fault_flags_t *faults);

/**
 * @brief Run training stage (certifiable-training)
 * @traceability CH-MATH-001 §3.2
 */
ch_result_t ch_stage_training(const ch_config_t *config,
                               ch_context_t *ctx,
                               ch_stage_result_t *result,
                               ch_fault_flags_t *faults);

/**
 * @brief Run quant stage (certifiable-quant)
 * @traceability CH-MATH-001 §3.3
 */
ch_result_t ch_stage_quant(const ch_config_t *config,
                            ch_context_t *ctx,
                            ch_stage_result_t *result,
                            ch_fault_flags_t *faults);

/**
 * @brief Run deploy stage (certifiable-deploy)
 * @traceability CH-MATH-001 §3.4
 */
ch_result_t ch_stage_deploy(const ch_config_t *config,
                             ch_context_t *ctx,
                             ch_stage_result_t *result,
                             ch_fault_flags_t *faults);

/**
 * @brief Run inference stage (certifiable-inference)
 * @traceability CH-MATH-001 §3.5
 */
ch_result_t ch_stage_inference(const ch_config_t *config,
                                ch_context_t *ctx,
                                ch_stage_result_t *result,
                                ch_fault_flags_t *faults);

/**
 * @brief Run monitor stage (certifiable-monitor)
 * @traceability CH-MATH-001 §3.6
 */
ch_result_t ch_stage_monitor(const ch_config_t *config,
                              ch_context_t *ctx,
                              ch_stage_result_t *result,
                              ch_fault_flags_t *faults);

/**
 * @brief Run verify stage (certifiable-verify)
 * @traceability CH-MATH-001 §3.7
 */
ch_result_t ch_stage_verify(const ch_config_t *config,
                             ch_context_t *ctx,
                             ch_stage_result_t *result,
                             ch_fault_flags_t *faults);

/**
 * @brief Initialize context
 */
void ch_context_init(ch_context_t *ctx);

/**
 * @brief Free context resources
 */
void ch_context_free(ch_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CH_STAGES_H */
