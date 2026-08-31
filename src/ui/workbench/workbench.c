/***************************************************************
**
** Nanokit Source File
**
** File         :  workbench.c
** Module       :  ui/workbench
** Author       :  SH
** Created      :  2026-05-16 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Workbench Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "ui/dock/dock.h"
#include <nanokit.h>
#include <backend/backend.h>

#include <stdio.h>
#include <ui/view/view.h>

#include <resource/resource.h>

#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/* Bootstrap Icons, UTF-8 encoded, drawn from the embedded icon font:
   layout-sidebar (U+F45F), window-dock (U+F61E) and layout-sidebar-reverse
   (U+F45E) — a panel on the left, along the bottom, and on the right. */
#define ICON_SIDEBAR_LEFT   "\xEF\x91\x9F"
#define ICON_SIDEBAR_BOTTOM "\xEF\x98\x9E"
#define ICON_SIDEBAR_RIGHT  "\xEF\x91\x9E"

#define TOGGLE_COUNT (3U)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static void toggle_pressed(nk_button_t *button);
static void refresh_toggle_state(nk_workbench_t *workbench);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void nk_workbench_init(nk_workbench_t *workbench, nk_workbench_create_info_t *create_info)
{
    workbench->titlebar.direction = NK_DIRECTION_HORIZONTAL;
    workbench->titlebar.height.type = NK_SIZING_FIXED;
    workbench->titlebar.height.value = 30;
    workbench->titlebar.align_y = NK_ALIGN_CENTER;
    workbench->titlebar.padding.left = 10;
    workbench->titlebar.padding.right = 6;
    #if __APPLE__
    /* Leave room for the traffic lights. */
    workbench->titlebar.padding.left = 80;
    #endif
    #if _WIN32
    /* Keep the area toggles clear of the DWM caption buttons, which the system
       draws over the right end of the titlebar. */
    workbench->titlebar.padding.right = 140;
    #endif
    workbench->titlebar.gap = 5;

    nk_dock_init(&workbench->dock);

    workbench->statusbar.direction = NK_DIRECTION_HORIZONTAL;
    workbench->statusbar.height.type = NK_SIZING_FIXED;
    workbench->statusbar.height.value = 25;
    workbench->statusbar.background_resource = NKRES_COLOR_BACKGROUND_TERTIARY;

    nk_view_add_child(&workbench->view, &workbench->titlebar);
    nk_view_add_child(&workbench->view, &workbench->dock.view);
    nk_view_add_child(&workbench->view, &workbench->statusbar);

    workbench->brand_label.view.type = NK_LABEL;
    workbench->brand_label.text = create_info->app_title;
    workbench->brand_label.text_info.color_resource = NKRES_COLOR_TEXT_PRIMARY;
    workbench->brand_label.text_info.size = 12.0f;
    workbench->brand_label.text_info.variant = NK_TEXT_BOLD;

    nk_view_add_child(&workbench->titlebar, &workbench->brand_label.view);

    workbench->menubar.view.width.type = NK_SIZING_FIT;

    nk_view_add_child(&workbench->titlebar, &workbench->menubar.view);

    if (create_info->menubar_create_info)
    {
        nk_menubar_init(&workbench->menubar, create_info->menubar_create_info);
    }

    /* Dock area toggles, pushed to the right edge by a growing spacer. */
    workbench->titlebar_spacer.type = NK_VIEW;
    workbench->titlebar_spacer.width.type = NK_SIZING_GROW;

    nk_view_add_child(&workbench->titlebar, &workbench->titlebar_spacer);

    static const char *toggle_icons[TOGGLE_COUNT] = {
        ICON_SIDEBAR_LEFT,
        ICON_SIDEBAR_BOTTOM,
        ICON_SIDEBAR_RIGHT
    };

    static const char *toggle_tooltips[TOGGLE_COUNT] = {
        "Toggle left sidebar",
        "Toggle bottom panel",
        "Toggle right sidebar"
    };

    for (size_t i = 0; i < TOGGLE_COUNT; i++)
    {
        nk_button_t *button = &workbench->area_toggles[i];

        /* nk_button_init derives the element id from the text, so the icon has
           to be in place before it runs. */
        button->text = toggle_icons[i];
        button->tooltip = toggle_tooltips[i];

        nk_button_init(button, toggle_pressed);

        button->text_info.variant = NK_TEXT_ICON;
        button->user_data = workbench;

        nk_view_add_child(&workbench->titlebar, &button->view);
    }

    refresh_toggle_state(workbench);

    #if 0
    const char* menu_items[] = {"File", "Edit", "View", "Help"};

    int num_menu_items = sizeof(menu_items) / sizeof(menu_items[0]);

    for (int i = 0; i < num_menu_items; i++)
    {
        nk_menu_t *menu = (nk_menu_t *)malloc(sizeof(nk_menu_t));
        memset(menu, 0, sizeof(nk_menu_t));
        menu->view.type = NK_MENU;
        menu->heading = menu_items[i];
        menu->view.id = menu_items[i];
        menu->view.width.type = NK_SIZING_FIT;
        menu->view.height.type = NK_SIZING_FIT;
        menu->view.padding.left = 5;
        menu->view.padding.right = 5;
        menu->view.padding.top = 2;
        menu->view.padding.bottom = 2;

        nk_view_t *popup = (nk_view_t *)malloc(sizeof(nk_view_t));
        memset(popup, 0, sizeof(nk_view_t));
        popup->type = NK_VIEW;
        popup->id = menu_items[i];

        nk_view_add_child(&menubar, &menu->view);
    }
    #endif
}

void workbench_render(nk_view_t *view)
{

}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

/* Which dock area each toggle drives, matching the order they are built in. */
static nk_dock_tab_location_t toggle_area(size_t index)
{
    switch (index)
    {
        case 0:  { return DOCK_TAB_LEFT_AREA;   }
        case 1:  { return DOCK_TAB_BOTTOM_AREA; }
        default: { return DOCK_TAB_RIGHT_AREA;  }
    }
}

/* A hidden area's icon is dimmed, so the row reads as three indicators of what
   is currently on screen rather than three identical buttons. */
static void refresh_toggle_state(nk_workbench_t *workbench)
{
    for (size_t i = 0; i < TOGGLE_COUNT; i++)
    {
        bool visible = nk_dock_is_area_visible(&workbench->dock, toggle_area(i));

        workbench->area_toggles[i].text_info.color_resource =
            visible ? NKRES_COLOR_TEXT_PRIMARY : NKRES_COLOR_TEXT_SECONDARY;
    }
}

static void toggle_pressed(nk_button_t *button)
{
    nk_workbench_t *workbench = (nk_workbench_t *)button->user_data;

    if (!workbench)
    {
        return;
    }

    for (size_t i = 0; i < TOGGLE_COUNT; i++)
    {
        if (&workbench->area_toggles[i] != button)
        {
            continue;
        }

        nk_dock_toggle_area(&workbench->dock, toggle_area(i));

        refresh_toggle_state(workbench);

        nk_window_request_redraw(backend_get_active_window());

        return;
    }
}
