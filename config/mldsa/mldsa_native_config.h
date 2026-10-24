/*
 * This file is part of the Pico Keys SDK distribution (https://github.com/polhenarejos/pico-keys-sdk).
 *
 * mldsa-native configuration for Pico Keys.
 *
 * Selected through MLD_CONFIG_FILE (an absolute path set by CMake) so that it
 * replaces the stock mldsa/mldsa_native_config.h for both the library build
 * and every consumer. It sets the Pico Keys options and then pulls in the
 * stock file for the defaults (parameter set, namespace, ...).
 *
 * Parameter set, namespace prefix and multilevel options come from CMake and
 * must not be defined here.
 */

#ifndef PICOKEYS_MLD_CONFIG_H
#define PICOKEYS_MLD_CONFIG_H

/* Portable C only: native backends stay off (none exist for Cortex-M33 or
 * Xtensa). MLD_CONFIG_NO_ASM is not set because the compiler barrier it would
 * remove is what makes the library's zeroization and constant-time value
 * barriers effective. */

/* No crypto_sign_* aliases: the three levels are linked together. */
#define MLD_CONFIG_NO_SUPERCOP

/* Keys are always derived from a stored 32-byte seed and the signing
 * randomness is drawn by the caller, so the library never needs randombytes(). */
#define MLD_CONFIG_NO_RANDOMIZED_API

/* Trade speed for RAM: large buffers shrink from ~108 KB to ~21 KB for
 * ML-DSA-87 signing. */
#define MLD_CONFIG_REDUCE_RAM

/* Move the remaining large buffers from the (small) card-thread stack to the
 * heap. MLD_FREE zeroizes before calling MLD_CUSTOM_FREE. */
#define MLD_CONFIG_CUSTOM_ALLOC_FREE
#if !defined(__ASSEMBLER__)
#include <stdint.h>
#include <stdlib.h>
static inline void *picokeys_mld_alloc(size_t align, size_t size) {
    /* Over-allocate and stash the original pointer just below the aligned
     * block, so this works on every libc (newlib, ESP-IDF, macOS, glibc). */
    uint8_t *raw = (uint8_t *)malloc(size + align + sizeof(void *));
    if (!raw) {
        return NULL;
    }
    uintptr_t aligned = ((uintptr_t)(raw + sizeof(void *)) + (align - 1)) & ~(uintptr_t)(align - 1);
    ((void **)aligned)[-1] = raw;
    return (void *)aligned;
}
static inline void picokeys_mld_free(void *ptr) {
    if (ptr) {
        free(((void **)ptr)[-1]);
    }
}
#define MLD_CUSTOM_ALLOC(v, T, N) T *v = (T *)picokeys_mld_alloc(MLD_DEFAULT_ALIGN, sizeof(T) * (N))
#define MLD_CUSTOM_FREE(v, T, N) picokeys_mld_free(v)
#endif /* !__ASSEMBLER__ */

/* Stock defaults (parameter set, namespace). Relative to this file, so the
 * same path works for the Pico, emulation and ESP-IDF builds. */
#include "../../third-party/mldsa/mldsa/mldsa_native_config.h"

#endif /* PICOKEYS_MLD_CONFIG_H */
