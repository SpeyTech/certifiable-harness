/**
 * @file hash.c
 * @brief Hash utilities
 * @traceability CH-MATH-001 §14
 *
 * Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
 */

#include "ch_hash.h"
#include "sha256.h"
#include <string.h>

ch_result_t ch_sha256(const uint8_t *data, size_t len,
                       uint8_t hash_out[CH_HASH_SIZE],
                       ch_fault_flags_t *faults)
{
    if (!hash_out || !faults) {
        return CH_ERR_NULL;
    }
    
    if (!data && len > 0) {
        return CH_ERR_NULL;
    }
    
    ct_sha256_ctx_t ctx;
    ct_sha256_init(&ctx);
    if (data && len > 0) {
        ct_sha256_update(&ctx, data, len);
    }
    ct_sha256_final(&ctx, hash_out);
    
    return CH_OK;
}

bool ch_hash_equal(const uint8_t a[CH_HASH_SIZE],
                    const uint8_t b[CH_HASH_SIZE])
{
    if (!a || !b) return false;
    
    /* Constant-time comparison */
    uint8_t diff = 0;
    for (int i = 0; i < CH_HASH_SIZE; i++) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

void ch_hash_copy(uint8_t dst[CH_HASH_SIZE],
                   const uint8_t src[CH_HASH_SIZE])
{
    if (dst && src) {
        memcpy(dst, src, CH_HASH_SIZE);
    }
}

void ch_hash_zero(uint8_t hash[CH_HASH_SIZE])
{
    if (hash) {
        memset(hash, 0, CH_HASH_SIZE);
    }
}

bool ch_hash_is_zero(const uint8_t hash[CH_HASH_SIZE])
{
    if (!hash) return true;
    
    for (int i = 0; i < CH_HASH_SIZE; i++) {
        if (hash[i] != 0) return false;
    }
    return true;
}

void ch_hash_to_hex(const uint8_t hash[CH_HASH_SIZE], char *buf)
{
    if (!hash || !buf) return;
    
    static const char hex[] = "0123456789abcdef";
    for (int i = 0; i < CH_HASH_SIZE; i++) {
        buf[i * 2] = hex[(hash[i] >> 4) & 0x0F];
        buf[i * 2 + 1] = hex[hash[i] & 0x0F];
    }
    buf[CH_HASH_SIZE * 2] = '\0';
}
