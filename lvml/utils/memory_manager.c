/**
 * @file memory_manager.c
 * @brief Memory management utilities implementation
 */

#include "memory_manager.h"
#include "py/mphal.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static bool memory_manager_initialized = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lvml_error_t memory_manager_init(void) {
    if (memory_manager_initialized) {
        return LVML_OK;
    }
    
    mp_printf(&mp_plat_print, "[MEMORY] Initializing memory manager\n");
    
    memory_manager_initialized = true;
    mp_printf(&mp_plat_print, "[MEMORY] Memory manager initialized successfully\n");
    
    return LVML_OK;
}

void memory_manager_deinit(void) {
    if (!memory_manager_initialized) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[MEMORY] Deinitializing memory manager\n");
    
    memory_manager_initialized = false;
    mp_printf(&mp_plat_print, "[MEMORY] Memory manager deinitialized\n");
}

void* memory_manager_alloc_aligned(size_t size, size_t alignment) {
    if (!memory_manager_initialized) {
        mp_printf(&mp_plat_print, "[MEMORY] Memory manager not initialized\n");
        return NULL;
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // For now, just use standard malloc
    // TODO: Implement proper aligned allocation if needed
    void* ptr = malloc(size);
    if (ptr != NULL) {
        mp_printf(&mp_plat_print, "[MEMORY] Allocated %d bytes at %p\n", size, ptr);
    } else {
        mp_printf(&mp_plat_print, "[MEMORY] Failed to allocate %d bytes\n", size);
    }
    
    return ptr;
}

void memory_manager_free_aligned(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    mp_printf(&mp_plat_print, "[MEMORY] Freeing memory at %p\n", ptr);
    free(ptr);
}

lvml_error_t memory_manager_get_stats(size_t* total, size_t* used, size_t* free) {
    if (!memory_manager_initialized) {
        mp_printf(&mp_plat_print, "[MEMORY] Memory manager not initialized\n");
        return LVML_ERROR_INIT;
    }
    
    if (total == NULL || used == NULL || free == NULL) {
        mp_printf(&mp_plat_print, "[MEMORY] Invalid parameters\n");
        return LVML_ERROR_INVALID_PARAM;
    }
    
    // TODO: Implement actual memory statistics
    // For now, return dummy values
    *total = 1024 * 1024; // 1MB
    *used = 512 * 1024;   // 512KB
    *free = 512 * 1024;   // 512KB
    
    mp_printf(&mp_plat_print, "[MEMORY] Memory stats: total=%d, used=%d, free=%d\n", 
              *total, *used, *free);
    
    return LVML_OK;
}

bool memory_manager_is_initialized(void) {
    return memory_manager_initialized;
}
