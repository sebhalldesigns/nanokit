/***************************************************************
**
** Nanokit Header File
**
** File         :  nanokit.h
** Module       :  nanokit
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Interface Definition
**
***************************************************************/

#ifndef NANOKIT_H
#define NANOKIT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef void (*nk_void_callback_t)(void);
typedef bool (*nk_bool_callback_t)(void);

typedef struct
{
    nk_void_callback_t launch_callback;
} nk_run_info_t;

typedef uintptr_t nk_window_t;

typedef struct
{
    const char* title;
    int start_width;
    int start_height;
} nk_window_create_info_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

/* run nanokit */
int nk_run(nk_run_info_t *info, int argc, char **argv);

bool nk_window_create(nk_window_create_info_t *info, nk_window_t *window);


#ifdef __cplusplus
}
#endif

#endif /* NANOKIT_H */
