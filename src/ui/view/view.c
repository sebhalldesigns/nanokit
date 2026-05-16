/***************************************************************
**
** Nanokit Source File
**
** File         :  view.c
** Module       :  ui/view
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit View Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

#include <resource/resource.h>
#include <ui/ui.h>

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

static void render_children(nk_view_t *parent);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void nk_view_add_child(nk_view_t *parent, nk_view_t *child)
{
    child->parent = parent;
    child->next_sibling = NULL;

    if (!parent->first_child)
    {
        child->prev_sibling = NULL;
        parent->first_child = child;
        return;
    }

    nk_view_t *last = parent->first_child;
    while (last->next_sibling)
    {
        last = last->next_sibling;
    }

    last->next_sibling = child;
    child->prev_sibling = last;
}

void nk_view_remove(nk_view_t *child)
{
    if (!child->parent)
    {
        return;
    }

    if (child->prev_sibling)
    {
        child->prev_sibling->next_sibling = child->next_sibling;
    }
    else
    {
        child->parent->first_child = child->next_sibling;
    }

    if (child->next_sibling)
    {
        child->next_sibling->prev_sibling = child->prev_sibling;
    }

    child->parent = NULL;
    child->prev_sibling = NULL;
    child->next_sibling = NULL;
}

void nk_view_insert_after(nk_view_t *sibling, nk_view_t *child)
{
    child->parent = sibling->parent;
    child->prev_sibling = sibling;
    child->next_sibling = sibling->next_sibling;

    if (sibling->next_sibling)
    {
        sibling->next_sibling->prev_sibling = child;
    }

    sibling->next_sibling = child;
}

void nk_view_insert_before(nk_view_t *sibling, nk_view_t *child)
{
    child->parent = sibling->parent;
    child->next_sibling = sibling;
    child->prev_sibling = sibling->prev_sibling;

    if (sibling->prev_sibling)
    {
        sibling->prev_sibling->next_sibling = child;
    }
    else if (sibling->parent)
    {
        sibling->parent->first_child = child;
    }

    sibling->prev_sibling = child;
}

void view_render(nk_view_t *view)
{
    switch (view->type)
    {

        case NK_LABEL:
        {
            label_render(view);
        }
        break;

        case NK_BUTTON:
        {
            button_render(view);
        }
        break;

        case NK_MENU:
        {
            menu_render(view);
        }
        break;

        case NK_MENUBAR:
        {
            menubar_render(view);
        }
        break;

        default:
        {
            CLAY({
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
        break;

    }
}


void view_render_children(nk_view_t *parent)
{
    nk_view_t *child = parent->first_child;
    while (child)
    {
        view_render(child);
        child = child->next_sibling;
    }
}


/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
