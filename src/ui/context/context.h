/***************************************************************
**
** Nanokit Header File
**
** File         :  context.h
** Module       :  ui/context
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit UI Context Interface Definition
**
***************************************************************/

#ifndef UI_CONTEXT_H
#define UI_CONTEXT_H

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

typedef uintptr_t ui_context_t;

bool ui_context_init(ui_context_t *context);

void ui_context_render(ui_context_t *context, nk_view_t *root, float width, float height, float dpr, float pointer_x, float pointer_y, bool mouse_down);

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* UI_CONTEXT_H */
