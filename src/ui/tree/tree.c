/***************************************************************
**
** Nanokit Source File
**
** File         :  tree.c
** Module       :  ui/tree
** Author       :  SH
** Created      :  2026-06-14 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Tree Item Implementation
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

#define TREE_ITEM_HEIGHT   (22.0f)
#define TREE_ITEM_INDENT   (14.0f)
#define TREE_ITEM_PADDING  (6.0f)
#define TREE_CHEVRON_WIDTH (10.0f)

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

void nk_tree_item_init(nk_tree_item_t *item, nk_tree_item_press_callback_t callback)
{
    item->view.type = NK_TREE_ITEM;
    item->view.id = ui_id_from_fmt("tree_item:%p", item);

    item->text_info.color_resource = NKRES_COLOR_TEXT_PRIMARY;
    item->text_info.size = resource_get_float(NKRES_SIZE_TEXT_PRIMARY);

    item->view.width.type = NK_SIZING_GROW;
    item->view.height.type = NK_SIZING_FIXED;
    item->view.height.value = TREE_ITEM_HEIGHT;

    item->depth = 0;
    item->is_folder = false;
    item->is_expanded = false;
    item->selected = false;

    item->user_data = NULL;
    item->press_callback = callback;
}

void tree_item_render(nk_view_t *view)
{
    nk_tree_item_t *item = (nk_tree_item_t *)view;

    if (!view->id) return;

    Clay_ElementId clay_id = { .id = view->id };

    bool hovered = Clay_PointerOver(clay_id);
    bool click = hovered && ui_pointer_press();

    if (click)
    {
        if (item->is_folder)
        {
            item->is_expanded = !item->is_expanded;
        }

        if (item->press_callback)
        {
            item->press_callback(item);
        }
    }

    nk_resource_t bg_res = item->selected ? NKRES_COLOR_BACKGROUND_SELECTED
                         : hovered        ? NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY
                         :                  NKRES_NONE;

    float left_padding = TREE_ITEM_PADDING + (float)item->depth * TREE_ITEM_INDENT;

    CLAY({
        .id = clay_id,
        .layout = {
            .sizing = {
                .width  = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height)
            },
            .padding = {
                .left   = (uint16_t)left_padding,
                .top    = 0,
                .right  = (uint16_t)TREE_ITEM_PADDING,
                .bottom = 0
            },
            .childGap = 4,
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
        .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(bg_res))
    })
    {
        /* Disclosure chevron — only folders get one; files reserve the space so
           their labels line up with their siblings. */
        CLAY({
            .layout = {
                .sizing = { .width = CLAY_SIZING_FIXED(TREE_CHEVRON_WIDTH) }
            }
        })
        {
            if (item->is_folder)
            {
                /* Bootstrap Icons: chevron-down U+F282 expanded, chevron-right
                   U+F285 collapsed (UTF-8 encoded), drawn from the icon font. */
                Clay_String chevron = item->is_expanded
                    ? (Clay_String){ .chars = "\xEF\x8A\x82", .length = 3 }
                    : (Clay_String){ .chars = "\xEF\x8A\x85", .length = 3 };

                CLAY_TEXT(chevron, CLAY_TEXT_CONFIG({
                    .fontSize  = (uint16_t)item->text_info.size,
                    .fontId    = NK_TEXT_ICON,
                    .textColor = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_TEXT_SECONDARY)),
                }));
            }
        }

        Clay_String label = { .chars = item->text, .length = (int32_t)strlen(item->text) };

        CLAY_TEXT(label, CLAY_TEXT_CONFIG({
            .fontSize  = (uint16_t)item->text_info.size,
            .textColor = nk_to_clay_color(resource_get_dynamic_color(item->text_info.color_resource)),
            .wrapMode  = CLAY_TEXT_WRAP_NONE,
        }));
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
