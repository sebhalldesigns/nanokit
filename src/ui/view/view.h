/***************************************************************
**
** Nanokit Header File
**
** File         :  view.h
** Module       :  ui/view
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit View Interface Definition
**
***************************************************************/

#ifndef VIEW_H
#define VIEW_H

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

void view_render(nk_view_t *view);
void view_render_children(nk_view_t *view);

void nk_view_insert_before(nk_view_t *sibling, nk_view_t *child);

#ifdef __cplusplus
}
#endif

#endif /* VIEW_H */
