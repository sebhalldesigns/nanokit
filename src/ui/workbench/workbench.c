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

#include <nanokit.h>

#include <stdio.h>
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

void nk_workbench_init(nk_workbench_t *workbench, nk_workbench_create_info_t *create_info)
{
    workbench->titlebar.direction = NK_DIRECTION_HORIZONTAL;
    workbench->titlebar.height.type = NK_SIZING_FIXED;
    workbench->titlebar.height.value = 30;
    workbench->titlebar.align_y = NK_ALIGN_CENTER;
    workbench->titlebar.padding.left = 10;
    #if __APPLE__
    workbench->titlebar.padding.left = 80;
    #endif
    workbench->titlebar.gap = 5;

    workbench->horizontal.direction = NK_DIRECTION_HORIZONTAL;
    workbench->horizontal.background_resource = NKRES_COLOR_BACKGROUND_PRIMARY;

    workbench->statusbar.direction = NK_DIRECTION_HORIZONTAL;
    workbench->statusbar.height.type = NK_SIZING_FIXED;
    workbench->statusbar.height.value = 25;
    workbench->statusbar.background_resource = NKRES_COLOR_BACKGROUND_TERTIARY;

    nk_view_add_child(&workbench->view, &workbench->titlebar);
    nk_view_add_child(&workbench->view, &workbench->horizontal);
    nk_view_add_child(&workbench->view, &workbench->statusbar);

    workbench->left_toolbar.width.type = NK_SIZING_FIXED;
    workbench->left_toolbar.width.value = 250;
    workbench->left_toolbar.background_resource = NKRES_COLOR_BACKGROUND_SECONDARY;

    workbench->main_content.background_resource = NKRES_COLOR_BACKGROUND_PRIMARY;

    workbench->right_toolbar.width.type = NK_SIZING_FIXED;
    workbench->right_toolbar.width.value = 250;
    workbench->horizontal.background_resource = NKRES_COLOR_BACKGROUND_SECONDARY;

    nk_view_add_child(&workbench->horizontal, &workbench->left_toolbar);
    nk_view_add_child(&workbench->horizontal, &workbench->main_content);
    nk_view_add_child(&workbench->horizontal, &workbench->right_toolbar);

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
