/***************************************************************
**
** Nanokit Header File
**
** File         :  scroll_view.h
** Module       :  ui/scroll_view
** Author       :  SH
** Created      :  2026-06-14 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Scroll View Interface Definition
**
***************************************************************/

#ifndef SCROLL_VIEW_H
#define SCROLL_VIEW_H

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

void scroll_view_render(nk_view_t *view);

#ifdef __cplusplus
}
#endif

#endif /* SCROLL_VIEW_H */
