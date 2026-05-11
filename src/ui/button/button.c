/***************************************************************
**
** Nanokit Source File
**
** File         :  button.c
** Module       :  ui/button
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Button Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

#include <ui/view/view.h>

#include <resource/resource.h>

#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/
void button_render(nk_view_t *view)
{
    nk_button_t *btn = (nk_button_t *)view;
    Clay_Color bg = nk_to_clay_color(resource_get_dynamic_color(view->background_resource));

    if (!view->id) return;

    Clay_ElementId clay_id = Clay_GetElementId((Clay_String){
        .chars = view->id,
        .length = (int32_t)strlen(view->id)
    });

    bool hovered = Clay_PointerOver(clay_id);

    if (hovered)
    {
        bg.r = (bg.r + 255) / 2;
        bg.g = (bg.g + 255) / 2;
        bg.b = (bg.b + 255) / 2;
        bg.a = 255;
    }

    Clay_String btn_str = { .chars = btn->text, .length = (int32_t)strlen(btn->text) };

    CLAY({
        .id = clay_id,
        .layout = {
            .sizing = {
                .width = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height)
            },
            .padding = {
                .left = view->padding.left,
                .top = view->padding.top,
                .right = view->padding.right,
                .bottom = view->padding.bottom
            }
        },
        .backgroundColor = bg,
        .cornerRadius = CLAY_CORNER_RADIUS(view->corner_radius)
    }) {
        CLAY_TEXT(btn_str, CLAY_TEXT_CONFIG({
            .fontSize = (uint16_t)btn->text_info.size,
            .textColor = nk_to_clay_color(resource_get_dynamic_color(btn->text_info.color_resource)),
        }));
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
