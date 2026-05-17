/***************************************************************
**
** Nanokit Header File
**
** File         :  dock_node.h
** Module       :  ui/dock
** Author       :  SH
** Created      :  2026-05-17 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Dock Node Interface Definition
**
***************************************************************/

#ifndef DOCK_NODE_H
#define DOCK_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef enum /* i.e location of b after split */
{
    SPLIT_DIRECTION_NONE,
    SPLIT_DIRECTION_LEFT,
    SPLIT_DIRECTION_RIGHT,
    SPLIT_DIRECTION_UP,
    SPLIT_DIRECTION_DOWN
} node_split_direction_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

void dock_node_init_group(nk_dock_node_t *node);

void dock_node_init_split(nk_dock_node_t *node, nk_dock_node_t *child_a,
    nk_dock_node_t *child_b, node_split_direction_t direction);

bool dock_node_split(nk_dock_node_t *existing_node, nk_dock_node_t *node_list,
    size_t list_size, node_split_direction_t direction, nk_dock_node_t **new_node);

void dock_node_render(nk_view_t *view);

#ifdef __cplusplus
}
#endif

#endif /* DOCK_NODE_H */
