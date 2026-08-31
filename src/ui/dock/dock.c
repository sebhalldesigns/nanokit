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

/* Smallest size, in pixels, a dock side area may be resized to. */
#define DOCK_AREA_MIN_SIZE (50.0f)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static void apply_area_visibility(nk_dock_t *dock);

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

static nk_dock_node_t *location_pool(nk_dock_t *dock,
    nk_dock_tab_location_t location, size_t *count)
{
    switch (location)
    {
        case DOCK_TAB_LEFT_AREA:
        {
            *count = NK_DOCK_NODES_PER_SIDE_AREA;
            return dock->left_nodes;
        }

        case DOCK_TAB_RIGHT_AREA:
        {
            *count = NK_DOCK_NODES_PER_SIDE_AREA;
            return dock->right_nodes;
        }

        case DOCK_TAB_BOTTOM_AREA:
        {
            *count = NK_DOCK_NODES_PER_SIDE_AREA;
            return dock->bottom_nodes;
        }

        case DOCK_TAB_MAIN_AREA:
        {
            *count = NK_DOCK_NODES_PER_MAIN_AREA;
            return dock->main_nodes;
        }
    }

    *count = 0;
    return NULL;
}

/* Which area a node's pool slot belongs to. */
static bool node_location(nk_dock_t *dock, const nk_dock_node_t *node,
    nk_dock_tab_location_t *location)
{
    for (size_t i = 0; i < NK_DOCK_TAB_LOCATION_COUNT; i++)
    {
        nk_dock_tab_location_t candidate = (nk_dock_tab_location_t)i;

        size_t count = 0;
        nk_dock_node_t *pool = location_pool(dock, candidate, &count);

        if (pool && node >= pool && node < pool + count)
        {
            *location = candidate;
            return true;
        }
    }

    return false;
}

/* The remembered group for an area, but only if it is still a live leaf there.
   Splits and collapses retire groups without telling anyone, so the pointer is
   confirmed against the pool rather than trusted. */
static nk_dock_group_t *recent_live_group(nk_dock_t *dock,
    nk_dock_tab_location_t location)
{
    nk_dock_group_t *group = (nk_dock_group_t *)dock->recent_groups[location];

    if (!group)
    {
        return NULL;
    }

    size_t count = 0;
    nk_dock_node_t *pool = location_pool(dock, location, &count);

    for (size_t i = 0; i < count; i++)
    {
        if (pool[i].active
            && pool[i].node_type == NK_DOCK_NODE_TYPE_LEAF
            && &pool[i].content.group == group)
        {
            return group;
        }
    }

    return NULL;
}

void dock_note_focus(nk_dock_group_t *group)
{
    if (!group || !group->node)
    {
        return;
    }

    nk_dock_node_t *node = (nk_dock_node_t *)group->node;

    if (!node->dock)
    {
        return;
    }

    nk_dock_t *dock = (nk_dock_t *)node->dock;

    nk_dock_tab_location_t location;

    if (!node_location(dock, node, &location))
    {
        return;
    }

    dock->recent_groups[location] = (struct nk_dock_group_t *)group;
}

void nk_dock_init(nk_dock_t *dock)
{
    /* Node pools must start fully inactive — slot reuse keys off `active`, and
       a garbage flag from a non-zeroed allocation corrupts split allocation. */
    memset(dock->left_nodes,   0, sizeof(dock->left_nodes));
    memset(dock->right_nodes,  0, sizeof(dock->right_nodes));
    memset(dock->bottom_nodes, 0, sizeof(dock->bottom_nodes));
    memset(dock->main_nodes,   0, sizeof(dock->main_nodes));
    memset(dock->recent_groups, 0, sizeof(dock->recent_groups));

    dock->view.type = NK_DOCK;
    dock->view.direction = NK_DIRECTION_HORIZONTAL;
    dock->view.background_resource = NKRES_COLOR_BACKGROUND_SECONDARY;

    dock->left_area.width.type = NK_SIZING_FIXED;
    dock->left_area.width.value = 250;

    nk_splitter_init(&dock->left_splitter, &dock->left_area.width.value, NK_DIRECTION_HORIZONTAL, false);
    nk_splitter_init(&dock->right_splitter, &dock->right_area.width.value, NK_DIRECTION_HORIZONTAL, true);

    dock->left_splitter.min  = DOCK_AREA_MIN_SIZE;
    dock->right_splitter.min = DOCK_AREA_MIN_SIZE;

    dock->right_area.width.type = NK_SIZING_FIXED;
    dock->right_area.width.value = 250;

    dock->central_area.direction = NK_DIRECTION_VERTICAL;

    nk_splitter_init(&dock->bottom_splitter, &dock->bottom_area.height.value, NK_DIRECTION_VERTICAL, true);

    dock->bottom_splitter.min = DOCK_AREA_MIN_SIZE;

    dock->bottom_area.height.type = NK_SIZING_FIXED;
    dock->bottom_area.height.value = 250;

    dock->left_visible = true;
    dock->right_visible = true;
    dock->bottom_visible = true;

    apply_area_visibility(dock);

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

/* Rebuild the dock's child lists in canonical order from the visibility flags.
   Rebuilding beats splicing a view back into the slot it came from: the order
   of these children *is* the layout, and there is no position left to restore
   once a view has been detached. */
static void apply_area_visibility(nk_dock_t *dock)
{
    nk_view_remove(&dock->left_area);
    nk_view_remove(&dock->left_splitter.view);
    nk_view_remove(&dock->central_area);
    nk_view_remove(&dock->right_splitter.view);
    nk_view_remove(&dock->right_area);

    if (dock->left_visible)
    {
        nk_view_add_child(&dock->view, &dock->left_area);
        nk_view_add_child(&dock->view, &dock->left_splitter.view);
    }

    nk_view_add_child(&dock->view, &dock->central_area);

    if (dock->right_visible)
    {
        nk_view_add_child(&dock->view, &dock->right_splitter.view);
        nk_view_add_child(&dock->view, &dock->right_area);
    }

    nk_view_remove(&dock->main_area);
    nk_view_remove(&dock->bottom_splitter.view);
    nk_view_remove(&dock->bottom_area);

    nk_view_add_child(&dock->central_area, &dock->main_area);

    if (dock->bottom_visible)
    {
        nk_view_add_child(&dock->central_area, &dock->bottom_splitter.view);
        nk_view_add_child(&dock->central_area, &dock->bottom_area);
    }
}

void nk_dock_set_area_visible(nk_dock_t *dock, nk_dock_tab_location_t area, bool visible)
{
    switch (area)
    {
        case DOCK_TAB_LEFT_AREA:   { dock->left_visible = visible;   } break;
        case DOCK_TAB_RIGHT_AREA:  { dock->right_visible = visible;  } break;
        case DOCK_TAB_BOTTOM_AREA: { dock->bottom_visible = visible; } break;

        /* The main area is the dock's reason to exist. */
        case DOCK_TAB_MAIN_AREA:   { return; }
    }

    apply_area_visibility(dock);
}

bool nk_dock_is_area_visible(const nk_dock_t *dock, nk_dock_tab_location_t area)
{
    switch (area)
    {
        case DOCK_TAB_LEFT_AREA:   { return dock->left_visible;   }
        case DOCK_TAB_RIGHT_AREA:  { return dock->right_visible;  }
        case DOCK_TAB_BOTTOM_AREA: { return dock->bottom_visible; }
        case DOCK_TAB_MAIN_AREA:   { return true; }
    }

    return false;
}

bool nk_dock_toggle_area(nk_dock_t *dock, nk_dock_tab_location_t area)
{
    bool visible = !nk_dock_is_area_visible(dock, area);

    nk_dock_set_area_visible(dock, area, visible);

    return nk_dock_is_area_visible(dock, area);
}

void nk_dock_add_tab(nk_dock_t *dock, nk_dock_tab_t *tab, nk_dock_tab_location_t location)
{
    /* Land the tab where the user last worked in this area, so opening a file
       while a split pane is focused puts it in that pane. */
    nk_dock_group_t *group = recent_live_group(dock, location);

    if (!group)
    {
        size_t count = 0;
        nk_dock_node_t *pool = location_pool(dock, location, &count);
        nk_dock_node_t *node = pool ? find_first_leaf(pool, count) : NULL;

        if (!node) { return; }

        group = &node->content.group;
    }

    tab->dock_group = (struct nk_dock_group_t *)group;
    tab->view.id = ui_id_from_fmt("dock_tab_btn:%p", tab);

    nk_view_add_child(&group->content, &tab->view);

    if (!group->active_tab)
    {
        group->active_tab = tab;
    }

    dock_note_focus(group);
}

/* Resolve the group a tab currently sits in from its position in the view
   tree. A docked tab is always a child of its group's content view, and that
   link is rewired everywhere a tab moves — including the group copy a split
   performs — which the tab's own dock_group back-pointer historically was not.
   Deriving it keeps focus working no matter how the tab got where it is. */
static nk_dock_group_t *group_of_tab(const nk_dock_tab_t *tab)
{
    if (!tab || !tab->view.parent)
    {
        return NULL;
    }

    return (nk_dock_group_t *)((char *)tab->view.parent
                               - offsetof(nk_dock_group_t, content));
}

void nk_dock_focus_tab(nk_dock_tab_t *tab)
{
    nk_dock_group_t *group = group_of_tab(tab);

    if (!group)
    {
        return;
    }

    group->active_tab = tab;

    dock_note_focus(group);
}

void nk_dock_close_tab(nk_dock_tab_t *tab)
{
    if (!tab || !tab->view.parent)
    {
        return;
    }

    nk_dock_group_t *group = group_of_tab(tab);

    nk_dock_tab_t *prev = (nk_dock_tab_t *)tab->view.prev_sibling;
    nk_dock_tab_t *next = (nk_dock_tab_t *)tab->view.next_sibling;

    nk_view_remove(&tab->view);

    /* The group must never be left pointing at a tab it no longer contains. */
    if (group && group->active_tab == tab)
    {
        group->active_tab = next ? next : prev;
    }
}

bool nk_dock_tab_is_open(const nk_dock_tab_t *tab)
{
    /* Closing a tab detaches its view from the group's content list, which is
       the only trace a caller holding the tab can observe. */
    return tab && tab->view.parent != NULL;
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
