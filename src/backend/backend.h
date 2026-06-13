/***************************************************************
**
** Nanokit Header File
**
** File         :  backend.h
** Module       :  backend
** Author       :  SH
** Created      :  2026-05-10 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Backend Interface Definition
**
***************************************************************/

#ifndef BACKEND_H
#define BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

bool backend_init(void);

int backend_run(nk_run_info_t *info, int argc, char **argv);

nk_window_t *backend_get_active_window(void);

#ifdef __cplusplus
}
#endif

#endif /* BACKEND_H */
