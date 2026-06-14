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

#include <ui/ui.h>

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

void nk_button_init(nk_button_t *button, nk_button_press_callback_t callback)
{
    button->view.type = NK_BUTTON;
    button->view.id = ui_id_from_fmt("button:%s", button->text);

    button->text_info.color_resource = NKRES_COLOR_TEXT_PRIMARY;
    button->text_info.size = resource_get_float(NKRES_SIZE_TEXT_PRIMARY);

    button->secondary_text_info.color_resource = NKRES_COLOR_TEXT_SECONDARY;
    button->secondary_text_info.size = resource_get_float(NKRES_SIZE_TEXT_PRIMARY);

    button->view.width.type = NK_SIZING_FIT;
    button->view.height.type = NK_SIZING_FIT;
    button->view.padding.left = 6;
    button->view.padding.right = 6;
    button->view.padding.top = 4;
    button->view.padding.bottom = 4;
    button->view.corner_radius = resource_get_float(NKRES_SIZE_BUTTON_CORNER_RADIUS);

    button->user_data = NULL;
    button->press_callback = callback;


}

void button_render(nk_view_t *view)
{

    nk_button_t *btn = (nk_button_t *)view;

    if (!view->id) return;

    Clay_ElementId clay_id = { .id = btn->view.id };

    bool hovered = Clay_PointerOver(clay_id);
    bool click = hovered && ui_pointer_press();

    if (hovered)
    {
        btn->view.background_resource = NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY;
    }
    else
    {
        btn->view.background_resource = NKRES_NONE;
    }


    if (click && btn->press_callback)
    {
        btn->press_callback(btn);
    }

    Clay_Color bg = nk_to_clay_color(resource_get_dynamic_color(view->background_resource));


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
            },

        },
        .backgroundColor = bg,
        .cornerRadius = CLAY_CORNER_RADIUS(view->corner_radius)
    }) {
        Clay_String btn_str = { .chars = btn->text, .length = (int32_t)strlen(btn->text) };

        CLAY_TEXT(btn_str, CLAY_TEXT_CONFIG({
            .fontSize = (uint16_t)btn->text_info.size,
            .textColor = nk_to_clay_color(resource_get_dynamic_color(btn->text_info.color_resource)),
        }));

        if (btn->secondary_text)
        {
            /* Spacer that eats all remaining horizontal space  */
            CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) {}

            Clay_String btn_str = { .chars = btn->secondary_text, .length = (int32_t)strlen(btn->secondary_text) };

            CLAY_TEXT(btn_str, CLAY_TEXT_CONFIG({
                .fontSize = (uint16_t)btn->secondary_text_info.size,
                .textColor = nk_to_clay_color(resource_get_dynamic_color(btn->secondary_text_info.color_resource)),
            }));
        }
    }

}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
