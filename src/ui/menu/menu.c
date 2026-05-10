/***************************************************************
**
** Nanokit Source File
**
** File         :  menu.c
** Module       :  ui/menu
** Author       :  SH
** Created      :  2026-05-10 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Menu Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

#include <ui/view/view.h>

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
    Clay_Color bg = nk_to_clay_color(view->background);

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
            .textColor = nk_to_clay_color(btn->text_info.color)
        }));

        // Floating tooltip anchored to this button
        if (hovered && btn->tooltip)
        {
            CLAY({
                .id = Clay_GetElementId(CLAY_STRING("tooltip")),
                .floating = {
                    .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                    .parentId = clay_id.id,
                    .attachPoints = {
                        .element = CLAY_ATTACH_POINT_LEFT_TOP,
                        .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
                    },
                    .offset = { .y = -4 },
                    .zIndex = 100,
                    .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH
                },
                .backgroundColor = { 255, 0, 0, 255 },
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(200),
                        .height = CLAY_SIZING_FIXED(40)
                    },
                    .padding = { 8, 6, 8, 6 }
                },
                .cornerRadius = CLAY_CORNER_RADIUS(4)
            }) {
                Clay_String tooltip_str = {
                    .chars = btn->tooltip,
                    .length = (int32_t)strlen(btn->tooltip)
                };
                CLAY_TEXT(tooltip_str, CLAY_TEXT_CONFIG({
                    .fontSize = 12,
                    .textColor = { 230, 230, 230, 255 }
                }));
            }
        }
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
