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

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
