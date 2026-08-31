/***************************************************************
**
** Nanokit Source File
**
** File         :  menubar.c
** Module       :  ui/menubar
** Author       :  SH
** Created      :  2026-05-16 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Menubar Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

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

void nk_menubar_init(nk_menubar_t *menubar, nk_menubar_create_info_t *create_info)
{

    menubar->view.type = NK_MENUBAR;
    menubar->view.id = ui_id_from_fmt("menubar:%p", (void *)menubar);
    menubar->view.direction = NK_DIRECTION_HORIZONTAL;
    menubar->view.align_y = NK_ALIGN_CENTER;
    menubar->view.gap = 5;

    for (int i = 0; i < create_info->menus_count; i++)
    {
        nk_menu_init(&create_info->menus[i], menubar);
        nk_view_add_child(&menubar->view, &create_info->menus[i].view);

        /* An entry that advertises a shortcut should honour it, so the chord
           printed beside the item is bound to the same command the item
           dispatches. */
        nk_menu_t *menu = &create_info->menus[i];

        for (size_t j = 0; j < menu->entries_count; j++)
        {
            nk_menu_entry_t *entry = &menu->entries[j];

            if (entry->is_separator || !entry->shortcut || !entry->command)
            {
                continue;
            }

            if (!nk_shortcut_register(entry->shortcut, entry->command))
            {
                fprintf(stderr, "nanokit: could not bind shortcut \"%s\" for %s\n",
                        entry->shortcut, entry->command);
            }
        }
    }
}


void menubar_render(nk_view_t *view)
{
     nk_menubar_t *menubar = (nk_menubar_t *)view;

     if (menubar->is_open)
     {
         assert(menubar->open_menu != NULL);

         Clay_ElementId menu_id = { .id = menubar->open_menu->view.id };
         Clay_ElementId popup_id = { .id = menubar->open_menu->popup.id };

        /* handle close by three different methods:
         *  1) release a new press outside of menu header or popup
         *  2) release a new press on the active header
         *  3) release the pointer over the popup area
         */
        if (!Clay_PointerOver(menu_id) && !Clay_PointerOver(popup_id) && ui_pointer_release() && !menubar->is_initial_press)
        {
            menubar->is_open = false;
        }
        else if (Clay_PointerOver(menu_id) && ui_pointer_release() && !menubar->is_initial_press)
        {
            menubar->is_open = false;
        }
        else if (Clay_PointerOver(popup_id) && ui_pointer_release())
        {
            menubar->is_open = false;
        }

        if (ui_pointer_release())
        {
            menubar->is_initial_press = false;
        }
     }

     /* While a menu is open the menubar owns input: a transparent, full-window
      * scrim sits above all content (but below the popups) and captures every
      * pointer event — hover, click and scroll — so nothing underneath reacts.
      * The strip occupied by the menubar itself is left uncovered so the menu
      * headers stay live for hover-to-switch and click-to-close. This is a
      * general modal layer, not tied to any particular widget beneath it. */
     if (menubar->is_open)
     {
         Clay_ElementData bar = Clay_GetElementData((Clay_ElementId){ .id = view->id });
         float bar_bottom = bar.found ? bar.boundingBox.y + bar.boundingBox.height : 0.0f;
         nk_point_t win = ui_window_size();

         CLAY({
             .id = CLAY_ID("menubar_scrim"),
             .floating = {
                 .attachTo = CLAY_ATTACH_TO_ROOT,
                 .attachPoints = {
                     .element = CLAY_ATTACH_POINT_LEFT_TOP,
                     .parent  = CLAY_ATTACH_POINT_LEFT_TOP
                 },
                 .offset = { .x = 0.0f, .y = bar_bottom },
                 .zIndex = 90,
                 .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_CAPTURE
             },
             .layout = {
                 .sizing = {
                     .width  = CLAY_SIZING_FIXED(win.x),
                     .height = CLAY_SIZING_FIXED(win.y - bar_bottom)
                 }
             }
         }) {}
     }

     CLAY({
         .id = { .id = view->id },
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
             .childAlignment = {
                 .x = (view->align_x == NK_ALIGN_CENTER) ? CLAY_ALIGN_X_CENTER
                     : (view->align_x == NK_ALIGN_END)    ? CLAY_ALIGN_X_RIGHT
                     :                                      CLAY_ALIGN_X_LEFT,
                 .y = (view->align_y == NK_ALIGN_CENTER) ? CLAY_ALIGN_Y_CENTER
                     : (view->align_y == NK_ALIGN_END)    ? CLAY_ALIGN_Y_BOTTOM
                     :                                      CLAY_ALIGN_Y_TOP
             },
             .childGap = view->gap,
             .layoutDirection = (view->direction == NK_DIRECTION_HORIZONTAL)
                 ? CLAY_LEFT_TO_RIGHT
                 : CLAY_TOP_TO_BOTTOM
         },
         .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(view->background_resource)),
         .cornerRadius = CLAY_CORNER_RADIUS(view->corner_radius)
     })
     {
         view_render_children(view);
     }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
