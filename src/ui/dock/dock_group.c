/***************************************************************
**
** Nanokit Source File
**
** File         :  dock_group.c
** Module       :  ui/dock
** Author       :  SH
** Created      :  2026-05-17 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Dock Group Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "ui/view/view.h"
#include "ui/dock/dock_node.h"
#include <nanokit.h>

#include <stdio.h>
#include <math.h>
#include <ui/ui.h>

#include <resource/resource.h>

#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#define DRAG_THRESHOLD      (5.0f)
#define SPLIT_EDGE_FRACTION (0.2f)
#define OVERLAY_ALPHA       (0x60)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef enum {
    DRAG_NONE,
    DRAG_INSERT,
    DRAG_SPLIT,
    DRAG_DROP
} dock_drag_event_t;

typedef enum {
    DRAG_SPLIT_LEFT,
    DRAG_SPLIT_RIGHT,
    DRAG_SPLIT_UP,
    DRAG_SPLIT_DOWN
} dock_drag_dir_t;

typedef struct {
    nk_dock_tab_t   *tab;
    nk_dock_group_t *source_group;
    nk_point_t       press_point;
    bool             active;

    nk_dock_group_t *target_group;
    dock_drag_event_t event_type;
    dock_drag_dir_t   split_dir;
    int               insert_index; /* -1 = append */

    nk_dock_tab_t   *close_tab;   /* tab whose close button was pressed, pending release */
    nk_dock_group_t *close_group;
} dock_drag_state_t;

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static dock_drag_state_t drag = {0};

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static float distance(nk_point_t a, nk_point_t b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return sqrtf(dx*dx + dy*dy);
}

static void group_remove_tab(nk_dock_group_t *group, nk_dock_tab_t *tab)
{
    bool was_active = (group->active_tab == tab);
    nk_dock_tab_t *prev = (nk_dock_tab_t *)tab->view.prev_sibling;
    nk_dock_tab_t *next = (nk_dock_tab_t *)tab->view.next_sibling;
    nk_view_remove(&tab->view);
    if (was_active)
        group->active_tab = next ? next : prev;
}

static void group_add_tab(nk_dock_group_t *group, nk_dock_tab_t *tab, int index)
{
    if (index < 0)
    {
        nk_view_add_child(&group->content, &tab->view);
    }
    else
    {
        nk_view_t *child = group->content.first_child;
        int i = 0;
        while (child && i < index) { child = child->next_sibling; i++; }

        if (child)
            nk_view_insert_before(child, &tab->view);
        else
            nk_view_add_child(&group->content, &tab->view);
    }
    tab->dock_group = (struct nk_dock_group_t *)group;
    group->active_tab = tab;
}

static node_split_direction_t drag_dir_to_split(dock_drag_dir_t dir)
{
    switch (dir)
    {
        case DRAG_SPLIT_LEFT:  { return SPLIT_DIRECTION_LEFT;  } break;
        case DRAG_SPLIT_RIGHT: { return SPLIT_DIRECTION_RIGHT; } break;
        case DRAG_SPLIT_UP:    { return SPLIT_DIRECTION_UP;    } break;
        case DRAG_SPLIT_DOWN:  { return SPLIT_DIRECTION_DOWN;  } break;
        default:               { return SPLIT_DIRECTION_NONE;  } break;
    }
}

static bool find_node_pool(nk_dock_t *dock, nk_dock_node_t *node,
                            nk_dock_node_t **pool_out, size_t *size_out)
{
    if (node >= dock->left_nodes && node < dock->left_nodes + NK_DOCK_NODES_PER_SIDE_AREA)
    {
        *pool_out = dock->left_nodes;
        *size_out = NK_DOCK_NODES_PER_SIDE_AREA;
        return true;
    }
    if (node >= dock->right_nodes && node < dock->right_nodes + NK_DOCK_NODES_PER_SIDE_AREA)
    {
        *pool_out = dock->right_nodes;
        *size_out = NK_DOCK_NODES_PER_SIDE_AREA;
        return true;
    }
    if (node >= dock->bottom_nodes && node < dock->bottom_nodes + NK_DOCK_NODES_PER_SIDE_AREA)
    {
        *pool_out = dock->bottom_nodes;
        *size_out = NK_DOCK_NODES_PER_SIDE_AREA;
        return true;
    }
    if (node >= dock->main_nodes && node < dock->main_nodes + NK_DOCK_NODES_PER_MAIN_AREA)
    {
        *pool_out = dock->main_nodes;
        *size_out = NK_DOCK_NODES_PER_MAIN_AREA;
        return true;
    }
    return false;
}

static bool node_is_tool_area(nk_dock_node_t *node)
{
    if (!node || !node->dock) { return false; }
    nk_dock_t      *dock = (nk_dock_t *)node->dock;
    nk_dock_node_t *pool = NULL;
    size_t          size = 0;
    if (!find_node_pool(dock, node, &pool, &size)) { return false; }
    return pool != dock->main_nodes;
}

/* Collapse a leaf node whose group has become empty.
   Replaces its parent split node with the sibling in the view tree.
   Does nothing if the leaf is a base-area root (parent is not a dock node). */
static void maybe_collapse_empty_node(nk_dock_node_t *leaf)
{
    if (!leaf) return;
    if (leaf->content.group.content.first_child != NULL) return;

    nk_view_t *parent_view = leaf->view.parent;
    if (!parent_view || parent_view->type != NK_DOCK_NODE) return;

    nk_dock_node_t *split = (nk_dock_node_t *)parent_view;
    if (split->node_type != NK_DOCK_NODE_TYPE_SPLIT) return;

    nk_dock_node_t *sibling = (split->content.split.child_a == leaf)
        ? split->content.split.child_b
        : split->content.split.child_a;
    if (!sibling) return;

    /* Replace split in the grandparent's child list with sibling */
    nk_view_t *grandparent = split->view.parent;
    nk_view_t *prev        = split->view.prev_sibling;
    nk_view_t *next        = split->view.next_sibling;

    if (prev) prev->next_sibling = &sibling->view;
    else if (grandparent) grandparent->first_child = &sibling->view;
    if (next) next->prev_sibling = &sibling->view;

    sibling->view.parent       = grandparent;
    sibling->view.prev_sibling = prev;
    sibling->view.next_sibling = next;
    sibling->view.width        = split->view.width;
    sibling->view.height       = split->view.height;

    split->active = false;
    leaf->active  = false;
}

static void execute_drop(void)
{
    nk_dock_tab_t   *tab = drag.tab;
    nk_dock_group_t *src = drag.source_group;
    nk_dock_group_t *dst = drag.target_group;

    if (!tab || !dst) return;

    /* Enforce area restrictions: tools → tool areas only, documents → main area only */
    if (tab->is_tool != node_is_tool_area((nk_dock_node_t *)dst->node)) return;

    if (drag.event_type == DRAG_DROP)
    {
        if (src == dst) { dst->active_tab = tab; return; }
        nk_dock_node_t *src_node = (nk_dock_node_t *)src->node;
        group_remove_tab(src, tab);
        maybe_collapse_empty_node(src_node);
        group_add_tab(dst, tab, -1);
        return;
    }

    if (drag.event_type == DRAG_INSERT)
    {
        /* Adjust index if removing from same group shifts the insertion point */
        int index = drag.insert_index;
        if (src == dst && index > 0)
        {
            int src_idx = 0;
            nk_view_t *tv = src->content.first_child;
            while (tv && tv != &tab->view) { src_idx++; tv = tv->next_sibling; }
            if (src_idx < index) index--;
        }
        group_remove_tab(src, tab);
        if (src != dst) maybe_collapse_empty_node((nk_dock_node_t *)src->node);
        group_add_tab(dst, tab, index);
        return;
    }

    if (drag.event_type == DRAG_SPLIT)
    {
        nk_dock_node_t *target_node = (nk_dock_node_t *)dst->node;
        if (!target_node || !target_node->dock) return;

        nk_dock_t      *dock      = (nk_dock_t *)target_node->dock;
        nk_dock_node_t *pool      = NULL;
        size_t          pool_size = 0;
        if (!find_node_pool(dock, target_node, &pool, &pool_size)) return;

        /* Detect whether source and target are in the same node BEFORE the split
           invalidates dst pointer */
        bool same_node = ((nk_dock_node_t *)src->node == target_node);

        nk_dock_node_t *new_leaf = NULL;
        nk_dock_node_t *copy     = NULL;
        if (!dock_node_split(target_node, pool, pool_size,
                             drag_dir_to_split(drag.split_dir),
                             &new_leaf, &copy)) return;

        /* Propagate dock back-pointer to the two fresh leaves */
        copy->dock     = (struct nk_dock_t *)dock;
        new_leaf->dock = (struct nk_dock_t *)dock;

        /* When dragging a tab from the same group we just split, remove it
           from the copy (which holds the original group's content), not from
           the now-invalid dst pointer. */
        nk_dock_group_t *effective_src = same_node ? &copy->content.group : src;

        group_remove_tab(effective_src, tab);
        maybe_collapse_empty_node((nk_dock_node_t *)effective_src->node);
        group_add_tab(&new_leaf->content.group, tab, -1);
    }
}

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void dock_group_init(nk_dock_group_t *group)
{
    memset(group, 0, sizeof(nk_dock_group_t));
    group->view.type = NK_DOCK_GROUP;
    group->view.id = ui_id_from_fmt("dock_group:%p", group);
    group->view.width.type = NK_SIZING_GROW;
    group->view.height.type = NK_SIZING_GROW;
    group->tab_bar.id = ui_id_from_fmt("dock_group_tabbar:%p", group);
    group->content.id = ui_id_from_fmt("dock_group_content:%p", group);
}

void dock_group_render(nk_view_t *view)
{
    nk_dock_group_t *group = (nk_dock_group_t *)view;

    Clay_ElementId clay_id    = { .id = group->view.id };
    Clay_ElementId tab_bar_id = { .id = group->tab_bar.id };
    Clay_ElementId content_id = { .id = group->content.id };

    float tab_font_size = resource_get_float(NKRES_SIZE_TEXT_PRIMARY);

    /* Close: execute pending close on pointer release */
    if (drag.close_tab && drag.close_group == group && ui_pointer_release())
    {
        nk_dock_node_t  *node = (nk_dock_node_t *)drag.close_group->node;
        nk_dock_group_t *cg   = drag.close_group;
        nk_dock_tab_t   *ct   = drag.close_tab;
        drag.close_tab   = NULL;
        drag.close_group = NULL;
        group_remove_tab(cg, ct);
        maybe_collapse_empty_node(node);
    }

    /* Drag: release clears state */
    if (drag.tab && ui_pointer_release())
    {
        if (drag.active)
        {
            execute_drop();
        }
        drag = (dock_drag_state_t){0};
    }

    /* Drag: threshold promotion */
    if (drag.tab && !drag.active && ui_pointer_down())
    {
        if (distance(ui_pointer_location(), drag.press_point) > DRAG_THRESHOLD)
        {
            drag.active = true;
        }
    }

    /* Drag: target detection — runs for every group each frame */
    if (drag.active)
    {
        nk_point_t ptr = ui_pointer_location();
        nk_rect_t  b   = group->content_bounds;

        /* Extend upward to include the tab bar (approx) */
        float full_y = b.y - tab_font_size - 12.0f; /* padding offsets */
        bool  over_group = (ptr.x >= b.x && ptr.x < b.x + b.width &&
                            ptr.y >= full_y && ptr.y < b.y + b.height);

        bool compatible = drag.tab &&
            (drag.tab->is_tool == node_is_tool_area((nk_dock_node_t *)group->node));

        if (over_group && compatible)
        {
            drag.target_group = group;
            bool in_content = (ptr.y >= b.y);

            if (!in_content)
            {
                /* Tab bar: find insert slot */
                drag.event_type   = DRAG_INSERT;
                drag.insert_index = -1;

                nk_view_t *tv = group->content.first_child;
                int        i  = 0;
                while (tv)
                {
                    Clay_ElementId tid = { .id = ((nk_dock_tab_t *)tv)->view.id };
                    Clay_ElementData td = Clay_GetElementData(tid);
                    if (td.found)
                    {
                        float mid = td.boundingBox.x + td.boundingBox.width * 0.5f;
                        if (ptr.x < mid)
                        {
                            drag.insert_index = i;
                            break;
                        }
                    }
                    i++;
                    tv = tv->next_sibling;
                }
            }
            else
            {
                /* Content area: determine split quadrant */
                float rx = (ptr.x - b.x) / b.width;
                float ry = (ptr.y - b.y) / b.height;

                if (rx < SPLIT_EDGE_FRACTION)
                {
                    drag.event_type = DRAG_SPLIT;
                    drag.split_dir  = DRAG_SPLIT_LEFT;
                }
                else if (rx > 1.0f - SPLIT_EDGE_FRACTION)
                {
                    drag.event_type = DRAG_SPLIT;
                    drag.split_dir  = DRAG_SPLIT_RIGHT;
                }
                else if (ry < SPLIT_EDGE_FRACTION)
                {
                    drag.event_type = DRAG_SPLIT;
                    drag.split_dir  = DRAG_SPLIT_UP;
                }
                else if (ry > 1.0f - SPLIT_EDGE_FRACTION)
                {
                    drag.event_type = DRAG_SPLIT;
                    drag.split_dir  = DRAG_SPLIT_DOWN;
                }
                else
                {
                    drag.event_type = DRAG_DROP;
                }
            }
        }
        else if (drag.target_group == group)
        {
            drag.target_group = NULL;
        }
    }

    /* Main layout */
    CLAY({
        .id = clay_id,
        .layout = {
            .sizing = {
                .width  = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height),
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
        },
        .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_BACKGROUND_PRIMARY)),
    })
    {
        /* Tab bar */
        CLAY({
            .id = tab_bar_id,
            .layout = {
                .sizing = {
                    .width  = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_FIT(0),
                },
                .layoutDirection = CLAY_LEFT_TO_RIGHT,
                .padding = { .left = 0, .right = 0, .top = 0, .bottom = 0 },
                .childGap = 0,
            },
            .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_BACKGROUND_SECONDARY)),
        })
        {
            bool is_insert_target = drag.active &&
                                    drag.target_group == group &&
                                    drag.event_type   == DRAG_INSERT;

            Clay_Color insert_color = { .r = 71, .g = 135, .b = 209, .a = 200 };

            Clay_Color close_hover_bg = nk_to_clay_color(
                resource_get_dynamic_color(NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY));

            nk_view_t *tab_view = group->content.first_child;
            int i = 0;
            while (tab_view)
            {
                nk_dock_tab_t *tab       = (nk_dock_tab_t *)tab_view;
                bool           is_active = (tab == group->active_tab);
                bool           has_close = !tab->is_tool;

                Clay_ElementId tab_btn_id = { .id = tab->view.id };
                Clay_ElementId close_id   = { .id = ui_id_from_fmt("dock_tab_close:%p", tab) };

                bool tab_hovered   = Clay_PointerOver(tab_btn_id);
                bool show_x        = has_close && (is_active || tab_hovered);
                bool close_hovered = show_x && Clay_PointerOver(close_id);

                /* Close button press — arm pending close, suppress drag */
                if (has_close && close_hovered && ui_pointer_press() && !drag.tab)
                {
                    drag.close_tab   = tab;
                    drag.close_group = group;
                }

                /* Drag start — only when pointer is not on close button */
                if (tab_hovered && !close_hovered && ui_pointer_press() && !drag.close_tab)
                {
                    drag.tab          = tab;
                    drag.source_group = group;
                    drag.press_point  = ui_pointer_location();
                    group->active_tab = tab;
                    is_active         = true;
                }

                /* Left insert indicator */
                if (is_insert_target && drag.insert_index == i)
                {
                    CLAY({
                        .layout = {
                            .sizing = { .width = CLAY_SIZING_FIXED(2), .height = CLAY_SIZING_GROW(0) },
                        },
                        .backgroundColor = insert_color,
                    }) {}
                }

                nk_resource_t bg_res = is_active ? NKRES_COLOR_BACKGROUND_PRIMARY
                                     : tab_hovered ? NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY
                                     :               NKRES_NONE;

                CLAY({
                    .id = tab_btn_id,
                    .layout = {
                        .sizing = {
                            .width  = CLAY_SIZING_FIT(0),
                            .height = CLAY_SIZING_FIT(0),
                        },
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .childAlignment  = { .y = CLAY_ALIGN_Y_CENTER },
                        .padding  = { .left = 8, .right = has_close ? 4 : 8, .top = 6, .bottom = 6 },
                        .childGap = has_close ? 4 : 0,
                    },
                    .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(bg_res)),
                })
                {
                    Clay_String title = {
                        .chars  = tab->title,
                        .length = (int32_t)strlen(tab->title)
                    };
                    CLAY_TEXT(title, CLAY_TEXT_CONFIG({
                        .fontSize  = (uint16_t)tab_font_size,
                        .textColor = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_TEXT_PRIMARY)),
                    }));

                    /* Close button — always reserve space for document tabs to keep width stable */
                    if (has_close)
                    {
                        CLAY({
                            .id = close_id,
                            .layout = {
                                .sizing = {
                                    .width  = CLAY_SIZING_FIXED(18),
                                    .height = CLAY_SIZING_FIXED(18),
                                },
                                .childAlignment = {
                                    .x = CLAY_ALIGN_X_CENTER,
                                    .y = CLAY_ALIGN_Y_CENTER,
                                },
                            },
                            .backgroundColor = close_hovered ? close_hover_bg : (Clay_Color){0},
                            .cornerRadius    = CLAY_CORNER_RADIUS(3),
                        })
                        {
                            if (show_x)
                            {
                                /* × U+00D7, UTF-8: 0xC3 0x97 */
                                Clay_String x_str = { .chars = "\xC3\x97", .length = 2 };
                                CLAY_TEXT(x_str, CLAY_TEXT_CONFIG({
                                    .fontSize  = (uint16_t)(tab_font_size + 1),
                                    .textColor = nk_to_clay_color(
                                        resource_get_dynamic_color(NKRES_COLOR_TEXT_PRIMARY)),
                                }));
                            }
                        }
                    }
                }

                i++;
                tab_view = tab_view->next_sibling;
            }

            /* Trailing insert indicator (append to end) */
            if (is_insert_target && drag.insert_index == -1)
            {
                CLAY({
                    .layout = {
                        .sizing = { .width = CLAY_SIZING_FIXED(2), .height = CLAY_SIZING_GROW(0) },
                    },
                    .backgroundColor = insert_color,
                }) {}
            }
        }

        /* Content area */
        CLAY({
            .id = content_id,
            .layout = {
                .sizing = {
                    .width  = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0),
                },
            },
            .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(NKRES_COLOR_BACKGROUND_PRIMARY)),
        })
        {
            /* Update content bounds for drag hit-testing next frame */
            Clay_ElementData cd = Clay_GetElementData(content_id);
            if (cd.found)
            {
                group->content_bounds.x      = cd.boundingBox.x;
                group->content_bounds.y      = cd.boundingBox.y;
                group->content_bounds.width  = cd.boundingBox.width;
                group->content_bounds.height = cd.boundingBox.height;
            }

            if (group->active_tab)
            {
                view_render_children(&group->active_tab->view);
            }
        }
    }

    /* Drop zone overlay (floating, on top of everything) */
    if (drag.active && drag.target_group == group && drag.event_type != DRAG_INSERT)
    {
        nk_rect_t b = group->content_bounds;

        float ox = b.x, oy = b.y, ow = b.width, oh = b.height;

        switch (drag.event_type == DRAG_SPLIT ? drag.split_dir : (dock_drag_dir_t)-1)
        {
            case DRAG_SPLIT_LEFT:
            {
                ow *= SPLIT_EDGE_FRACTION;
            } break;

            case DRAG_SPLIT_RIGHT:
            {
                ox += ow * (1.0f - SPLIT_EDGE_FRACTION);
                ow *= SPLIT_EDGE_FRACTION;
            } break;

            case DRAG_SPLIT_UP:
            {
                oh *= SPLIT_EDGE_FRACTION;
            } break;

            case DRAG_SPLIT_DOWN:
            {
                oy += oh * (1.0f - SPLIT_EDGE_FRACTION);
                oh *= SPLIT_EDGE_FRACTION;
            } break;

            default: { /* DRAG_DROP: full content area */ } break;
        }

        Clay_Color overlay = { .r = 71, .g = 135, .b = 209, .a = OVERLAY_ALPHA };

        CLAY({
            .floating = {
                .attachTo = CLAY_ATTACH_TO_ROOT,
                .offset   = { .x = ox, .y = oy },
                .zIndex   = 10,
                .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
            },
            .layout = {
                .sizing = {
                    .width  = CLAY_SIZING_FIXED(ow),
                    .height = CLAY_SIZING_FIXED(oh),
                },
            },
            .backgroundColor = overlay,
        }) {}
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
