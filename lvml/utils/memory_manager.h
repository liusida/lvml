/**
 * @file memory_manager.h
 * @brief Memory management utilities
 */

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "core/lvml_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      DEFINES
 *********************/

#define MEMORY_ALIGNMENT 4

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize memory manager
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t memory_manager_init(void);

/**
 * Deinitialize memory manager
 */
void memory_manager_deinit(void);

/**
 * Allocate aligned memory
 * @param size Size to allocate
 * @param alignment Alignment requirement
 * @return Pointer to allocated memory or NULL on failure
 */
void* memory_manager_alloc_aligned(size_t size, size_t alignment);

/**
 * Free aligned memory
 * @param ptr Pointer to free
 */
void memory_manager_free_aligned(void* ptr);

/**
 * Get memory usage statistics
 * @param total Total memory available
 * @param used Memory currently used
 * @param free Memory currently free
 * @return LVML_OK on success, error code on failure
 */
lvml_error_t memory_manager_get_stats(size_t* total, size_t* used, size_t* free);

/**
 * Check if memory manager is initialized
 * @return true if initialized, false otherwise
 */
bool memory_manager_is_initialized(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*MEMORY_MANAGER_H*/
