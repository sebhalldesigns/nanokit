/***************************************************************
**
** Nanokit Header File
**
** File         :  ui.h
** Module       :  ui/ui
** Author       :  SH
** Created      :  2026-05-16 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Base UI Interface
**
***************************************************************/

#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

#include <clay/clay.h>

#include <ui/view/view.h>

#include <ui/label/label.h>
#include <ui/button/button.h>
#include <ui/label/label.h>

#include <ui/menubar/menubar.h>
#include <ui/menubar/menu.h>

#include <ui/splitter/splitter.h>
#include <ui/dock/dock.h>
#include <ui/dock/dock_node.h>
#include <ui/dock/dock_group.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#define ID_MAX_STRING_LENGTH (512U)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef uintptr_t ui_context_t;

typedef struct
{
    const char* default_font;
    const char* bold_font;
} ui_context_create_info_t;

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
} ui_context_render_info_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

uint32_t ui_id_from_fmt(const char* message, ...);

/* true if pointer pressed this frame */
bool ui_pointer_press(void);

/* true if pointer release this frame */
bool ui_pointer_release(void);

nk_point_t ui_pointer_location(void);

Clay_Color nk_to_clay_color(nk_color_t c);

Clay_SizingAxis nk_to_clay_sizing(nk_sizing_t s);

bool ui_context_init(ui_context_t *context, ui_context_create_info_t *create_info);

void ui_context_render(ui_context_t *context, nk_view_t *root, ui_context_render_info_t *render_info);


#ifdef __cplusplus
}
#endif

#endif /* UI_H */
