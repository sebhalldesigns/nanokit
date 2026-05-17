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

void dock_node_init_group(nk_dock_node_t *node)
{
    memset(node, 0, sizeof(nk_dock_node_t));
    node->view.type = NK_DOCK_NODE;
    node->content.group.view.background_resource = NKRES_COLOR_RED;
    nk_view_add_child(&node->view, &node->content.group.view);
    node->active = true;
    node->node_type = NK_DOCK_NODE_TYPE_LEAF;
}

void dock_node_init_split(nk_dock_node_t *node, nk_dock_node_t *child_a,
    nk_dock_node_t *child_b, node_split_direction_t direction)
{
    memset(node, 0, sizeof(nk_dock_node_t));
    node->view.type = NK_DOCK_NODE;
    node->active = true;
    node->node_type = NK_DOCK_NODE_TYPE_SPLIT;
    node->view.id = ui_id_from_fmt("dock_node:%p", node);

    switch (direction)
    {
        case SPLIT_DIRECTION_UP:
        {
            child_b->view.height.type = NK_SIZING_PERCENT;
            child_b->view.height.value = 0.5f;
            nk_view_add_child(&node->view, &child_b->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_b->view.height.value, NK_DIRECTION_VERTICAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_a->view.height.type = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_a->view);

            node->content.split.direction = NK_DIRECTION_VERTICAL;
            node->view.direction = NK_DIRECTION_VERTICAL;
        } break;

        case SPLIT_DIRECTION_DOWN:
        {
            child_a->view.height.type = NK_SIZING_PERCENT;
            child_a->view.height.value = 0.5f;
            nk_view_add_child(&node->view, &child_a->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_a->view.height.value, NK_DIRECTION_VERTICAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_b->view.height.type = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_b->view);

            node->content.split.direction = NK_DIRECTION_VERTICAL;
            node->view.direction = NK_DIRECTION_VERTICAL;
        } break;

        case SPLIT_DIRECTION_LEFT:
        {
            child_b->view.width.type = NK_SIZING_PERCENT;
            child_b->view.width.value = 0.5f;
            nk_view_add_child(&node->view, &child_b->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_b->view.width.value, NK_DIRECTION_HORIZONTAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_a->view.width.type = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_a->view);

            node->content.split.direction = NK_DIRECTION_HORIZONTAL;
            node->view.direction = NK_DIRECTION_HORIZONTAL;
        } break;

        case SPLIT_DIRECTION_RIGHT:
        {
            child_a->view.width.type = NK_SIZING_PERCENT;
            child_a->view.width.value = 0.5f;
            nk_view_add_child(&node->view, &child_a->view);

            nk_splitter_init_proportional(&node->content.split.splitter,
                &child_a->view.width.value, NK_DIRECTION_HORIZONTAL, false,
                &node->content.split.axis_reference
            );

            nk_view_add_child(&node->view, &node->content.split.splitter.view);

            child_b->view.width.type = NK_SIZING_GROW;
            nk_view_add_child(&node->view, &child_b->view);

            node->content.split.direction = NK_DIRECTION_HORIZONTAL;
            node->view.direction = NK_DIRECTION_HORIZONTAL;
        } break;

        default:
        {

        } break;
    }
}

bool dock_node_split(nk_dock_node_t *existing_node, nk_dock_node_t *node_list,
    size_t list_size, node_split_direction_t direction, nk_dock_node_t **new_node)
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
            if (!new_a)
            {
                new_a = &node_list[i];
            }
            else if (!new_b)
            {
                new_b = &node_list[i];
            }
            else
            {
                break;
            }
        }
    }

    if (!new_a || !new_b) return false;

    /* copy current to new_a */
    memcpy(new_a, existing_node, sizeof(nk_dock_node_t));
    /* rewire interior pointers invalidated by the copy */
    new_a->view.first_child = &new_a->content.group.view;
    new_a->content.group.view.parent = &new_a->view;

    /* init new_b as new group */
    dock_node_init_group(new_b);

    /* setup existing as a split */
    dock_node_init_split(existing_node, new_a, new_b, direction);

    if (new_node)
    {
        *new_node = new_b;
    }

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
