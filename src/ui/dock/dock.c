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
#include "ui/dock/dock_group.h"
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

static void set_dock_backpointers(nk_dock_node_t *nodes, size_t count, nk_dock_t *dock)
{
    for (size_t i = 0; i < count; i++)
    {
        if (!nodes[i].active) { continue; }
        nodes[i].dock = (struct nk_dock_t *)dock;
        if (nodes[i].node_type == NK_DOCK_NODE_TYPE_LEAF)
        {
            nodes[i].content.group.node = &nodes[i];
        }
    }
}

static nk_dock_node_t *find_first_leaf(nk_dock_node_t *nodes, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (nodes[i].active && nodes[i].node_type == NK_DOCK_NODE_TYPE_LEAF)
        {
            return &nodes[i];
        }
    }
    return NULL;
}

void nk_dock_init(nk_dock_t *dock)
{
    /* Node pools must start fully inactive — slot reuse keys off `active`, and
       a garbage flag from a non-zeroed allocation corrupts split allocation. */
    memset(dock->left_nodes,   0, sizeof(dock->left_nodes));
    memset(dock->right_nodes,  0, sizeof(dock->right_nodes));
    memset(dock->bottom_nodes, 0, sizeof(dock->bottom_nodes));
    memset(dock->main_nodes,   0, sizeof(dock->main_nodes));

    dock->view.type = NK_DOCK;
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

    set_dock_backpointers(dock->left_nodes,   NK_DOCK_NODES_PER_SIDE_AREA, dock);
    set_dock_backpointers(dock->right_nodes,  NK_DOCK_NODES_PER_SIDE_AREA, dock);
    set_dock_backpointers(dock->bottom_nodes, NK_DOCK_NODES_PER_SIDE_AREA, dock);
    set_dock_backpointers(dock->main_nodes,   NK_DOCK_NODES_PER_MAIN_AREA, dock);
}

void nk_dock_add_tab(nk_dock_t *dock, nk_dock_tab_t *tab, nk_dock_tab_location_t location)
{
    nk_dock_node_t *node = NULL;

    switch (location)
    {
        case DOCK_TAB_LEFT_AREA:
        {
            node = find_first_leaf(dock->left_nodes, NK_DOCK_NODES_PER_SIDE_AREA);
        } break;

        case DOCK_TAB_RIGHT_AREA:
        {
            node = find_first_leaf(dock->right_nodes, NK_DOCK_NODES_PER_SIDE_AREA);
        } break;

        case DOCK_TAB_BOTTOM_AREA:
        {
            node = find_first_leaf(dock->bottom_nodes, NK_DOCK_NODES_PER_SIDE_AREA);
        } break;

        case DOCK_TAB_MAIN_AREA:
        {
            node = find_first_leaf(dock->main_nodes, NK_DOCK_NODES_PER_MAIN_AREA);
        } break;
    }

    if (!node) { return; }

    nk_dock_group_t *group = &node->content.group;
    tab->dock_group = (struct nk_dock_group_t *)group;
    tab->view.id = ui_id_from_fmt("dock_tab_btn:%p", tab);

    nk_view_add_child(&group->content, &tab->view);

    if (!group->active_tab)
    {
        group->active_tab = tab;
    }
}

void dock_render(nk_view_t *view)
{
    CLAY({
        .layout = {
            .sizing = {
                .width = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height),
            },
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
        },
        .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(view->background_resource)),
    })
    {
        view_render_children(view);
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
