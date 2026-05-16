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
    CLAY_TEXT(str, CLAY_TEXT_CONFIG({
        .fontSize = (uint16_t)label->text_info.size,
        .textColor = nk_to_clay_color(resource_get_dynamic_color(label->text_info.color_resource)),
        .fontId = label->text_info.variant
    }));
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
