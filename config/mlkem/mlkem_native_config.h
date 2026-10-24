/*
 * This file is part of the Pico Keys SDK distribution (https://github.com/polhenarejos/pico-keys-sdk).
 *
 * mlkem-native configuration for Pico Keys.
 *
 * Selected through MLK_CONFIG_FILE (an absolute path set by CMake) so that it
 * replaces the stock mlkem/mlkem_native_config.h for both the library build
 * and every consumer. It sets the Pico Keys options and then pulls in the
 * stock file for the defaults (parameter set, namespace, ...).
 *
 * Parameter set, namespace prefix and multilevel options come from CMake and
 * must not be defined here.
 */

#ifndef PICOKEYS_MLK_CONFIG_H
#define PICOKEYS_MLK_CONFIG_H

/* Portable C only: native backends stay off (none exist for Cortex-M33 or
 * Xtensa). MLK_CONFIG_NO_ASM is not set because the compiler barrier it would
 * remove is what makes the library's zeroization and constant-time value
 * barriers effective. */

/* No crypto_kem_* aliases: the three levels are linked together. */
#define MLK_CONFIG_NO_SUPERCOP

/* Keys are always derived from a stored 64-byte seed (d || z) and
 * encapsulation coins are supplied by the caller, so the library never needs
 * randombytes(). */
#define MLK_CONFIG_NO_RANDOMIZED_API

/* Move large buffers from the (small) card-thread stack to the heap.
 * MLK_FREE zeroizes before calling MLK_CUSTOM_FREE. */
#define MLK_CONFIG_CUSTOM_ALLOC_FREE
#if !defined(__ASSEMBLER__)
#include <stdint.h>
#include <stdlib.h>
static inline void *picokeys_mlk_alloc(size_t align, size_t size) {
    uint8_t *raw = (uint8_t *)malloc(size + align + sizeof(void *));
    if (!raw) {
        return NULL;
    }
    uintptr_t aligned = ((uintptr_t)(raw + sizeof(void *)) + (align - 1)) & ~(uintptr_t)(align - 1);
    ((void **)aligned)[-1] = raw;
    return (void *)aligned;
}
static inline void picokeys_mlk_free(void *ptr) {
    if (ptr) {
        free(((void **)ptr)[-1]);
    }
}
#define MLK_CUSTOM_ALLOC(v, T, N) T *v = (T *)picokeys_mlk_alloc(MLK_DEFAULT_ALIGN, sizeof(T) * (N))
#define MLK_CUSTOM_FREE(v, T, N) picokeys_mlk_free(v)
#endif /* !__ASSEMBLER__ */

/* Stock defaults (parameter set, namespace). Relative to this file, so the
 * same path works for the Pico, emulation and ESP-IDF builds. */
#include "../../third-party/mlkem/mlkem/mlkem_native_config.h"

#endif /* PICOKEYS_MLK_CONFIG_H */
