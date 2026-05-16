/***************************************************************
**
** Nanokit Header File
**
** File         :  workbench.h
** Module       :  ui/workbench
** Author       :  SH
** Created      :  2026-05-16 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Workbench Interface Definition
**
***************************************************************/

#ifndef WORKBENCH_H
#define WORKBENCH_H

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

void workbench_render(nk_view_t *view);

#ifdef __cplusplus
}
#endif

#endif /* WORKBENCH_H */
