/***************************************************************
**
** Nanokit Source File
**
** File         :  view.c
** Module       :  ui/view
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit View Implementation
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

void label_render(nk_view_t *view)
{
    nk_label_t *label = (nk_label_t *)view;
    Clay_String str = { .chars = label->text, .length = (int32_t)strlen(label->text) };

    Clay_TextElementConfig *config = CLAY_TEXT_CONFIG({
        .fontSize = (uint16_t)label->text_info.size,
        .textColor = nk_to_clay_color(resource_get_dynamic_color(label->text_info.color_resource)),
        .fontId = label->text_info.variant,
        .wrapMode = label->no_wrap ? CLAY_TEXT_WRAP_NONE : CLAY_TEXT_WRAP_WORDS
    });

    /* A bare label sizes itself to its text. Only when the caller has pinned an
       axis do we wrap it in a box that enforces that size — a fixed row height
       is what lets a virtualised scroll view predict where each label lands. */
    bool sized = (view->width.type == NK_SIZING_FIXED)
              || (view->height.type == NK_SIZING_FIXED);

    if (!sized)
    {
        CLAY_TEXT(str, config);
        return;
    }

    CLAY({
        .layout = {
            .sizing = {
                .width  = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height)
            },
            .padding = {
                .left   = view->padding.left,
                .top    = view->padding.top,
                .right  = view->padding.right,
                .bottom = view->padding.bottom
            },
            .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
        .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(view->background_resource))
    })
    {
        CLAY_TEXT(str, config);
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
