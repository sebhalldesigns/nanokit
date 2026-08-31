/***************************************************************
**
** Nanokit Source File
**
** File         :  dock_node.c
** Module       :  ui/dock
** Author       :  SH
** Created      :  2026-05-17 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Dock Node Implementation
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

/* Smallest size, in pixels, either pane of a split tab group may shrink to. */
#define DOCK_NODE_MIN_SIZE (50.0f)

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

void dock_node_init_group(nk_dock_node_t *node)
{
    memset(node, 0, sizeof(nk_dock_node_t));
    node->view.type = NK_DOCK_NODE;
    node->active = true;
    node->node_type = NK_DOCK_NODE_TYPE_LEAF;
    dock_group_init(&node->content.group);
    node->content.group.node = node;
    nk_view_add_child(&node->view, &node->content.group.view);
}

void dock_node_init_split(nk_dock_node_t *node, nk_dock_node_t *child_a,
    nk_dock_node_t *child_b, node_split_direction_t direction)
{
    /* Preserve position in the view hierarchy so grandparent stays wired */
    nk_view_t        *saved_parent = node->view.parent;
    nk_view_t        *saved_prev   = node->view.prev_sibling;
    nk_view_t        *saved_next   = node->view.next_sibling;
    nk_sizing_t       saved_width  = node->view.width;
    nk_sizing_t       saved_height = node->view.height;
    struct nk_dock_t *saved_dock   = node->dock;

    memset(node, 0, sizeof(nk_dock_node_t));

    node->view.parent       = saved_parent;
    node->view.prev_sibling = saved_prev;
    node->view.next_sibling = saved_next;
    node->view.width        = saved_width;
    node->view.height       = saved_height;
    node->dock              = saved_dock;

    node->view.type = NK_DOCK_NODE;
    node->active = true;
    node->node_type = NK_DOCK_NODE_TYPE_SPLIT;
    node->view.id = ui_id_from_fmt("dock_node:%p", node);

    switch (direction)
    {
        case SPLIT_DIRECTION_UP:
        {
            child_b->view.width.type  = NK_SIZING_GROW;
            child_b->view.height.type = NK_SIZING_PERCENT;
            child_b->view.height.value = 0.5f;
            nk_view_add_child(&node->view, &child_b->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_b->view.height.value, NK_DIRECTION_VERTICAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_a->view.width.type  = NK_SIZING_GROW;
            child_a->view.height.type = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_a->view);

            node->content.split.direction = NK_DIRECTION_VERTICAL;
            node->view.direction = NK_DIRECTION_VERTICAL;
            node->content.split.child_a = child_a;
            node->content.split.child_b = child_b;
        } break;

        case SPLIT_DIRECTION_DOWN:
        {
            child_a->view.width.type  = NK_SIZING_GROW;
            child_a->view.height.type = NK_SIZING_PERCENT;
            child_a->view.height.value = 0.5f;
            nk_view_add_child(&node->view, &child_a->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_a->view.height.value, NK_DIRECTION_VERTICAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_b->view.width.type  = NK_SIZING_GROW;
            child_b->view.height.type = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_b->view);

            node->content.split.direction = NK_DIRECTION_VERTICAL;
            node->view.direction = NK_DIRECTION_VERTICAL;
            node->content.split.child_a = child_a;
            node->content.split.child_b = child_b;
        } break;

        case SPLIT_DIRECTION_LEFT:
        {
            child_b->view.height.type = NK_SIZING_GROW;
            child_b->view.width.type  = NK_SIZING_PERCENT;
            child_b->view.width.value = 0.5f;
            nk_view_add_child(&node->view, &child_b->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_b->view.width.value, NK_DIRECTION_HORIZONTAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_a->view.height.type = NK_SIZING_GROW;
            child_a->view.width.type  = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_a->view);

            node->content.split.direction = NK_DIRECTION_HORIZONTAL;
            node->view.direction = NK_DIRECTION_HORIZONTAL;
            node->content.split.child_a = child_a;
            node->content.split.child_b = child_b;
        } break;

        case SPLIT_DIRECTION_RIGHT:
        {
            child_a->view.height.type = NK_SIZING_GROW;
            child_a->view.width.type  = NK_SIZING_PERCENT;
            child_a->view.width.value = 0.5f;
            nk_view_add_child(&node->view, &child_a->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_a->view.width.value, NK_DIRECTION_HORIZONTAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_b->view.height.type = NK_SIZING_GROW;
            child_b->view.width.type  = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_b->view);

            node->content.split.direction = NK_DIRECTION_HORIZONTAL;
            node->view.direction = NK_DIRECTION_HORIZONTAL;
            node->content.split.child_a = child_a;
            node->content.split.child_b = child_b;
        } break;

        default:
        {

        } break;
    }

    /* Bound the split so neither tab group can be collapsed away (and overflow
       onto its neighbours) by dragging the divider fully one way. */
    if (direction != SPLIT_DIRECTION_NONE)
    {
        node->content.split.splitter.min = DOCK_NODE_MIN_SIZE;
    }
}

bool dock_node_split(nk_dock_node_t *existing_node, nk_dock_node_t *node_list,
    size_t list_size, node_split_direction_t direction,
    nk_dock_node_t **new_node, nk_dock_node_t **copy_node)
{
    assert(existing_node);
    assert(node_list);

    if (direction == SPLIT_DIRECTION_NONE) return false;

    nk_dock_node_t *new_a = NULL;
    nk_dock_node_t *new_b = NULL;

    for (size_t i = 0; i < list_size; i++)
    {
        if (!node_list[i].active)
        {
            if (!new_a)      new_a = &node_list[i];
            else if (!new_b) new_b = &node_list[i];
            else             break;
        }
    }

    if (!new_a || !new_b) return false;

    /* copy existing content to new_a */
    memcpy(new_a, existing_node, sizeof(nk_dock_node_t));

    /* rewire view pointers invalidated by the copy */
    new_a->view.first_child             = &new_a->content.group.view;
    new_a->content.group.view.parent    = &new_a->view;
    new_a->content.group.view.prev_sibling = NULL;
    new_a->content.group.view.next_sibling = NULL;
    new_a->content.group.node           = new_a;

    /* fix Clay IDs which are keyed on the group pointer address */
    new_a->content.group.view.id    = ui_id_from_fmt("dock_group:%p",        &new_a->content.group);
    new_a->content.group.tab_bar.id = ui_id_from_fmt("dock_group_tabbar:%p", &new_a->content.group);
    new_a->content.group.content.id = ui_id_from_fmt("dock_group_content:%p",&new_a->content.group);

    /* fix tab parent and group back-pointers, which still reference the old
       group — whose storage dock_node_init_split() is about to reuse as the
       split's own fields, so anything left pointing at it writes into the
       split. */
    nk_view_t *tv = new_a->content.group.content.first_child;
    while (tv)
    {
        nk_dock_tab_t *tab = (nk_dock_tab_t *)tv;

        tv->parent      = &new_a->content.group.content;
        tab->dock_group = (struct nk_dock_group_t *)&new_a->content.group;

        tv = tv->next_sibling;
    }

    /* clear position pointers — nk_view_add_child will rewire them */
    new_a->view.parent       = NULL;
    new_a->view.prev_sibling = NULL;
    new_a->view.next_sibling = NULL;

    /* init new_b as an empty leaf */
    dock_node_init_group(new_b);

    /* turn existing_node into a split containing new_a and new_b */
    dock_node_init_split(existing_node, new_a, new_b, direction);

    if (new_node)  *new_node  = new_b;
    if (copy_node) *copy_node = new_a;

    return true;
}

void dock_node_render(nk_view_t *view)
{
    nk_dock_node_t *node = (nk_dock_node_t *)view;

    Clay_ElementId clay_id = { .id = node->view.id };

    if (node->node_type == NK_DOCK_NODE_TYPE_SPLIT)
    {
        Clay_ElementData data = Clay_GetElementData(clay_id);

        if (node->content.split.direction == NK_DIRECTION_HORIZONTAL)
        {
            node->content.split.axis_reference = data.boundingBox.width;
        }
        else
        {
            node->content.split.axis_reference = data.boundingBox.height;
        }

        node->content.split.axis_reference -= resource_get_float(NKRES_SIZE_SPLITTER_THICKNESS);

    }


    CLAY({
        .id = clay_id,
        .layout = {
            .sizing = {
                .width = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height)
            },
            .layoutDirection = (view->direction == NK_DIRECTION_HORIZONTAL)
                ? CLAY_LEFT_TO_RIGHT
                : CLAY_TOP_TO_BOTTOM
        }
    })
    {
        view_render_children(view);
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
