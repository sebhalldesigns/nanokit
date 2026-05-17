/***************************************************************
**
** Nanokit Source File
**
** File         :  splitter.c
** Module       :  ui/splitter
** Author       :  SH
** Created      :  2026-05-17 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Splitter Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

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

void nk_splitter_init(nk_splitter_t *splitter, float *target, nk_direction_t direction, bool reversed)
{
    assert(splitter);
    assert(target);

    splitter->view.type = NK_SPLITTER;
    splitter->view.id = ui_id_from_fmt("splitter:%p:%d:%d", target, direction, reversed);

    splitter->target = target;
    splitter->direction = direction;
    splitter->reversed = reversed;
    splitter->sizing = false;

    if (direction == NK_DIRECTION_HORIZONTAL)
    {
        splitter->view.width.type = NK_SIZING_FIXED;
        splitter->view.width.value = resource_get_float(NKRES_SIZE_SPLITTER_THICKNESS);
    }
    else
    {
        splitter->view.height.type = NK_SIZING_FIXED;
        splitter->view.height.value = resource_get_float(NKRES_SIZE_SPLITTER_THICKNESS);
    }

    splitter->view.background_resource = NKRES_COLOR_BACKGROUND_SPLITTER_INACTIVE;

}

void nk_splitter_init_proportional(nk_splitter_t *splitter, float *target, nk_direction_t direction,
    bool reversed, float *reference)
{
    assert(reference);

    nk_splitter_init(splitter, target, direction, reversed);
    splitter->proportional = true;
    splitter->reference = reference;
}

void splitter_render(nk_view_t *view)
{
    nk_splitter_t *splitter = (nk_splitter_t *)view;

    Clay_ElementId clay_id = { .id = splitter->view.id };

    bool hovered = Clay_PointerOver(clay_id);

    if (hovered)
    {
        splitter->view.background_resource = NKRES_COLOR_BACKGROUND_SPLITTER_ACTIVE;
    }
    else
    {
        splitter->view.background_resource = NKRES_COLOR_BACKGROUND_SPLITTER_INACTIVE;
    }

    Clay_Color bg = nk_to_clay_color(resource_get_dynamic_color(view->background_resource));

    if (hovered && ui_pointer_press())
    {
        splitter->sizing = true;
        splitter->start_point = ui_pointer_location();

        splitter->start_target_size = *splitter->target;

    }
    else if (splitter->sizing && ui_pointer_release())
    {
        splitter->sizing = false;
    }

    if (splitter->sizing)
    {
        nk_point_t new_location = ui_pointer_location();

        float reversed = splitter->reversed ? -1.0f : 1.0f;

        if (splitter->direction == NK_DIRECTION_HORIZONTAL && !splitter->proportional)
        {
            float dx = new_location.x - splitter->start_point.x;
            *splitter->target = splitter->start_target_size + (dx * reversed);
        }
        else if (!splitter->proportional)
        {
            float dy = new_location.y - splitter->start_point.y;
            *splitter->target = splitter->start_target_size + (dy * reversed);
        }
        else if (splitter->direction == NK_DIRECTION_HORIZONTAL && *splitter->reference > 0.0f)
        {
            float dx = (new_location.x - splitter->start_point.x) / *splitter->reference;
            *splitter->target = splitter->start_target_size + (dx * reversed );
        }
        else if (*splitter->reference > 0.0f)
        {
            float dy = (new_location.y - splitter->start_point.y) / *splitter->reference;
            *splitter->target = splitter->start_target_size + (dy * reversed);
        }
    }

    CLAY({
        .id = clay_id,
        .layout = {
            .sizing = {
                .width = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height)
            },
        },
        .backgroundColor = bg,
        .cornerRadius = CLAY_CORNER_RADIUS(view->corner_radius)
    });
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
