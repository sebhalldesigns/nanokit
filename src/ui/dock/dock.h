/***************************************************************
**
** Nanokit Header File
**
** File         :  dock.h
** Module       :  ui/dock
** Author       :  SH
** Created      :  2026-05-17 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Dock Interface Definition
**
***************************************************************/

#ifndef DOCK_H
#define DOCK_H

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

void dock_render(nk_view_t *view);

/* Record `group` as the most recently focused one in its area, which is where
   nk_dock_add_tab() will place the next tab for that area. */
void dock_note_focus(nk_dock_group_t *group);



#ifdef __cplusplus
}
#endif

#endif /* DOCK_H */
