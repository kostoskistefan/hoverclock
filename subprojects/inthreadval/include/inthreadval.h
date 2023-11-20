#ifndef INTHREADVAL_H
#define INTHREADVAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum inthreadval_status_e
{
    INTHREADVAL_STATUS_OK,
    INTHREADVAL_STATUS_ERROR_ALREADY_STARTED,
    INTHREADVAL_STATUS_ERROR_NOT_STARTED,
} inthreadval_status_e;

typedef struct inthreadval_s inthreadval_s;

/**
 * @brief Creates a new Inthreadval instance
 * @return The newly created instance
 **/
extern inthreadval_s *inthreadval_create(void (*callback)(void), uint32_t ms_interval);

/**
 * @brief Destroys a Inthreadval instance and frees memory
 **/
extern void inthreadval_destroy(inthreadval_s *const inthreadval);

/**
 * @brief Creates a new thread and starts running the user defined function at a specified interval
 * @return Whether the thread is already running (INTHREADVAL_STATUS_ERROR_ALREADY_STARTED) or it was
 *successfully started (INTHREADVAL_STATUS_OK)
 **/
extern inthreadval_status_e inthreadval_start(inthreadval_s *const inthreadval);

/**
 * @brief Stops a created thread from running
 * @return Whether the thread is already stopped (INTHREADVAL_STATUS_ERROR_NOT_STARTED) or it was
 *successfully stopped (INTHREADVAL_STATUS_OK)
 **/
extern inthreadval_status_e inthreadval_stop(inthreadval_s *const inthreadval);

#ifdef __cplusplus
}
#endif

#endif // INTHREADVAL_H
