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

#include "ui/view/view.h"
#include <nanokit.h>

#include <stdio.h>
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

void nk_menu_init(nk_menu_t *menu, nk_menubar_t *menubar)
{
    static char id_buffer[ID_MAX_STRING_LENGTH];

    menu->view.type = NK_MENU;
    menu->view.id = ui_id_from_fmt("menu:%s", menu->heading);
    menu->view.width.type = NK_SIZING_FIT;
    menu->view.height.type = NK_SIZING_FIT;
    menu->view.padding.left = 6;
    menu->view.padding.right = 6;
    menu->view.padding.top = 4;
    menu->view.padding.bottom = 4;
    menu->view.corner_radius = resource_get_float(NKRES_SIZE_BUTTON_CORNER_RADIUS);

    menu->popup.type = NK_VIEW;
    menu->popup.id = ui_id_from_fmt("menu/popup:%s", menu->heading);
    menu->popup.height.type = NK_SIZING_GROW;
    menu->popup.gap = 2.0f;

    for (int i = 0; i < menu->entries_count; i++)
    {
        menu->entries[i].button.view.type = NK_LABEL;
        menu->entries[i].button.text = menu->entries[i].title;
        menu->entries[i].button.button_type = NK_BUTTON_SECONDARY;

        menu->entries[i].button.secondary_text = menu->entries[i].shortcut;

        nk_button_init(&menu->entries[i].button);
        menu->entries[i].button.view.width.type = NK_SIZING_GROW;

        nk_view_add_child(&menu->popup, &menu->entries[i].button.view);
    }

    menu->parent_menubar = menubar;
}

void menu_render(nk_view_t *view)
{
    nk_menu_t *menu = (nk_menu_t *)view;

    if (!view->id) return;

    Clay_ElementId clay_id = { .id = menu->view.id };

    Clay_ElementId popup_id = { .id = menu->popup.id };

    bool hovered_header = Clay_PointerOver(clay_id);

    bool hovered_either = Clay_PointerOver(clay_id) || Clay_PointerOver(popup_id);

    bool popup_open = menu->parent_menubar->is_open && menu->parent_menubar->open_menu == menu;

    if (hovered_header || popup_open)
    {
        menu->view.background_resource = NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY;
    }
    else
    {
        menu->view.background_resource = NKRES_NONE;
    }

    Clay_Color bg = nk_to_clay_color(resource_get_dynamic_color(view->background_resource));

    Clay_String btn_str = { .chars = menu->heading, .length = (int32_t)strlen(menu->heading) };

    /* capture opening of menubar  */
    if (hovered_header && ui_pointer_press() && !menu->parent_menubar->is_open)
    {
        menu->parent_menubar->is_open = true;
        menu->parent_menubar->open_menu = menu;
        menu->parent_menubar->is_initial_press = true;
    }
    /* capture switch to this menu  */
    else if (menu->parent_menubar->is_open
        && hovered_header
        && menu->parent_menubar->open_menu != menu)
   {
       menu->parent_menubar->open_menu = menu;
   }

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
            .fontSize = (uint16_t)resource_get_float(NKRES_SIZE_TEXT_PRIMARY),
            .textColor = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_TEXT_PRIMARY))
        }));

        /* Floating tooltip anchored to this button  */
        if (popup_open)
        {
            CLAY({
                .id = popup_id,
                .floating = {
                    .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                    .parentId = clay_id.id,
                    .attachPoints = {
                        .element = CLAY_ATTACH_POINT_LEFT_TOP,
                        .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM
                    },
                    .offset = { .y = 0, .x = 0 },
                    .zIndex = 100,
                    .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH
                },
                .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_BACKGROUND_TERTIARY)),
                .border = {
                    .color = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_BACKGROUND_POPUP)),
                    .width = CLAY_BORDER_OUTSIDE(1)
                },
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_FIXED(200),
                        .height = CLAY__SIZING_TYPE_FIT
                    },
                    .padding = { 3, 3, 3, 3 },
                },
                .cornerRadius = CLAY_CORNER_RADIUS(resource_get_float(NKRES_SIZE_POPUP_CORNER_RADIUS))
            }) {
                view_render(&menu->popup);
            }
        }
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
