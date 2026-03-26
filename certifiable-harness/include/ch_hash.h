/**
 * @file ch_hash.h
 * @brief Hash utilities and domain separation tags
 * @traceability CH-MATH-001 14
 *
 * Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#ifndef CH_HASH_H
#define CH_HASH_H

#include "ch_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Domain Separation Tags
 * @traceability CH-MATH-001 14
 *============================================================================*/

#define CH_TAG_GOLDEN   "CH:GOLDEN:v1"
#define CH_TAG_CONFIG   "CH:CONFIG:v1"
#define CH_TAG_REPORT   "CH:REPORT:v1"
#define CH_TAG_STAGE    "CH:STAGE:v1"

/*============================================================================
 * Hash Functions
 *============================================================================*/

/**
 * @brief Compute SHA-256 hash
 */
ch_result_t ch_sha256(const uint8_t *data, size_t len,
                       uint8_t hash_out[CH_HASH_SIZE],
                       ch_fault_flags_t *faults);

/**
 * @brief Compare two hashes (constant-time)
 */
bool ch_hash_equal(const uint8_t a[CH_HASH_SIZE],
                    const uint8_t b[CH_HASH_SIZE]);

/**
 * @brief Copy hash
 */
void ch_hash_copy(uint8_t dst[CH_HASH_SIZE],
                   const uint8_t src[CH_HASH_SIZE]);

/**
 * @brief Zero hash
 */
void ch_hash_zero(uint8_t hash[CH_HASH_SIZE]);

/**
 * @brief Check if hash is zero
 */
bool ch_hash_is_zero(const uint8_t hash[CH_HASH_SIZE]);

/**
 * @brief Format hash as hex string
 * @param hash Input hash
 * @param buf Output buffer (must be at least 65 bytes)
 */
void ch_hash_to_hex(const uint8_t hash[CH_HASH_SIZE], char *buf);

#ifdef __cplusplus
}
#endif

#endif /* CH_HASH_H */
