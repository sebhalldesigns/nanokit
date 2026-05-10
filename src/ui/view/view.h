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

#include <clay/clay.h>

#include <ui/button/button.h>
#include <ui/label/label.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef uintptr_t view_context_t;

typedef struct
{
    const char* default_font;
    const char* bold_font;
} view_context_create_info_t;

typedef struct
{
    float width;
    float height;
    float offset_y;
    float dpr;
    bool dark_mode;
    float pointer_x;
    float pointer_y;
    bool mouse_down;
} view_context_render_info_t;


/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

bool view_context_init(view_context_t *context, view_context_create_info_t *create_info);

void view_context_render(view_context_t *context, nk_view_t *root, view_context_render_info_t *render_info);

Clay_Color nk_to_clay_color(nk_color_t c);

Clay_SizingAxis nk_to_clay_sizing(nk_sizing_t s);


#ifdef __cplusplus
}
#endif

#endif /* VIEW_H */
