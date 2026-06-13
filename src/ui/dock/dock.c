/***************************************************************
**
** Nanokit Source File
**
** File         :  dock.c
** Module       :  ui/dock
** Author       :  SH
** Created      :  2026-05-17 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Dock Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "ui/dock/dock_node.h"
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

void nk_dock_init(nk_dock_t *dock)
{
    dock->view.direction = NK_DIRECTION_HORIZONTAL;
    dock->view.background_resource = NKRES_COLOR_BACKGROUND_SECONDARY;

    dock->left_area.width.type = NK_SIZING_FIXED;
    dock->left_area.width.value = 250;

    nk_splitter_init(&dock->left_splitter, &dock->left_area.width.value, NK_DIRECTION_HORIZONTAL, false);
    nk_splitter_init(&dock->right_splitter, &dock->right_area.width.value, NK_DIRECTION_HORIZONTAL, true);

    dock->right_area.width.type = NK_SIZING_FIXED;
    dock->right_area.width.value = 250;

    nk_view_add_child(&dock->view, &dock->left_area);
    nk_view_add_child(&dock->view, &dock->left_splitter.view);
    nk_view_add_child(&dock->view, &dock->central_area);
    nk_view_add_child(&dock->view, &dock->right_splitter.view);
    nk_view_add_child(&dock->view, &dock->right_area);

    dock->central_area.direction = NK_DIRECTION_VERTICAL;

    nk_splitter_init(&dock->bottom_splitter, &dock->bottom_area.height.value, NK_DIRECTION_VERTICAL, true);

    dock->bottom_area.height.type = NK_SIZING_FIXED;
    dock->bottom_area.height.value = 250;

    nk_view_add_child(&dock->central_area, &dock->main_area);
    nk_view_add_child(&dock->central_area, &dock->bottom_splitter.view);
    nk_view_add_child(&dock->central_area, &dock->bottom_area);

    dock_node_init_group(&dock->left_nodes[0]);
    nk_view_add_child(&dock->left_area, &dock->left_nodes[0].view);
    dock_node_init_group(&dock->right_nodes[0]);
    nk_view_add_child(&dock->right_area, &dock->right_nodes[0].view);
    dock_node_init_group(&dock->bottom_nodes[0]);
    nk_view_add_child(&dock->bottom_area, &dock->bottom_nodes[0].view);
    dock_node_init_group(&dock->main_nodes[0]);
    nk_view_add_child(&dock->main_area, &dock->main_nodes[0].view);

    nk_dock_node_t *new_node;
    if (dock_node_split(&dock->left_nodes[0], dock->left_nodes, NK_DOCK_NODES_PER_SIDE_AREA, SPLIT_DIRECTION_DOWN, &new_node))
    {
        dock_node_split(new_node, dock->left_nodes, NK_DOCK_NODES_PER_SIDE_AREA, SPLIT_DIRECTION_LEFT, NULL);
    }
}

void nk_dock_add_tab(nk_dock_t *dock, nk_dock_tab_t *tab, nk_dock_tab_location_t location)
{

}

void dock_render(nk_view_t *view)
{

}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
