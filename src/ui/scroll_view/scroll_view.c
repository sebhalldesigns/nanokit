/***************************************************************
**
** Nanokit Source File
**
** File         :  scroll_view.c
** Module       :  ui/scroll_view
** Author       :  SH
** Created      :  2026-06-14 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Scroll View Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

#include <ui/ui.h>

#include <resource/resource.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#define SCROLLBAR_INSET     (2.0f)
#define SCROLLBAR_MIN_THUMB (24.0f)

#define SCROLL_AXIS_VERTICAL   (0)
#define SCROLL_AXIS_HORIZONTAL (1)

/* Extra rows rendered above and below the viewport to hide pop-in. */
#define VIRTUAL_BUFFER (3)

/* Viewport height assumed before Clay has reported real geometry (first frame
   after a content change). Generous so no visible row is skipped. */
#define VIRTUAL_FALLBACK_VIEWPORT (1000.0f)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static struct
{
    uint32_t id;
    bool active;
    int axis;
    float grab_offset; /* pointer minus thumb start at grab time */
} scrollbar_drag = {0};

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static size_t count_children(nk_view_t *view);
static float thumb_length(float track, float content);
static float clamp01(float value);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void nk_scroll_view_init(nk_scroll_view_t *scroll_view)
{
    scroll_view->view.type = NK_SCROLL_VIEW;
    scroll_view->view.id = ui_id_from_fmt("scroll_view:%p", scroll_view);

    scroll_view->view.width.type = NK_SIZING_GROW;
    scroll_view->view.height.type = NK_SIZING_GROW;
    scroll_view->view.direction = NK_DIRECTION_VERTICAL;

    scroll_view->virtualize = false;
    scroll_view->item_height = 0.0f;

    scroll_view->geom_valid = false;
}

void scroll_view_render(nk_view_t *view)
{
    nk_scroll_view_t *scroll_view = (nk_scroll_view_t *)view;

    if (!view->id) return;

    Clay_ElementId clay_id = { .id = view->id };

    /* Previous-frame scroll geometry drives both virtualisation and the
       scrollbars. scrollPosition is Clay's live offset — safe to mutate. */
    Clay_ScrollContainerData scroll_data = Clay_GetScrollContainerData(clay_id);
    bool have_geometry = scroll_data.found && scroll_data.scrollPosition;

    Clay_ElementData box = Clay_GetElementData(clay_id);

    /* Resolve the geometry that drives virtualisation and the scrollbars. When
       the query succeeds, cache it; when it momentarily fails (see geom_valid in
       the struct) reuse the cache so the view doesn't flicker; only the genuine
       first frame, before any geometry exists, uses the generous fallback. */
    float viewport_h, viewport_w, content_h, content_w, scroll_y, scroll_x;

    if (have_geometry)
    {
        viewport_h = scroll_data.scrollContainerDimensions.height;
        viewport_w = scroll_data.scrollContainerDimensions.width;
        content_h  = scroll_data.contentDimensions.height;
        content_w  = scroll_data.contentDimensions.width;
        scroll_y   = -scroll_data.scrollPosition->y;
        scroll_x   = -scroll_data.scrollPosition->x;

        scroll_view->geom_viewport_h = viewport_h;
        scroll_view->geom_viewport_w = viewport_w;
        scroll_view->geom_content_h  = content_h;
        scroll_view->geom_content_w  = content_w;
        scroll_view->geom_scroll_y   = scroll_y;
        scroll_view->geom_scroll_x   = scroll_x;
        scroll_view->geom_valid      = true;
    }
    else if (scroll_view->geom_valid)
    {
        viewport_h = scroll_view->geom_viewport_h;
        viewport_w = scroll_view->geom_viewport_w;
        content_h  = scroll_view->geom_content_h;
        content_w  = scroll_view->geom_content_w;
        scroll_y   = scroll_view->geom_scroll_y;
        scroll_x   = scroll_view->geom_scroll_x;
    }
    else
    {
        viewport_h = VIRTUAL_FALLBACK_VIEWPORT;
        viewport_w = 0.0f;
        content_h  = 0.0f;
        content_w  = 0.0f;
        scroll_y   = 0.0f;
        scroll_x   = 0.0f;
    }

    bool show_vbar = content_h > viewport_h + 0.5f;
    bool show_hbar = content_w > viewport_w + 0.5f;

    float max_scroll_y = content_h - viewport_h;
    float max_scroll_x = content_w - viewport_w;

    float thickness = resource_get_float(NKRES_SIZE_SCROLLBAR_THICKNESS);

    /* Shorten each track by the other bar's footprint so they never meet in the
       corner. */
    float track_y = viewport_h - (show_hbar ? thickness + SCROLLBAR_INSET : 0.0f);
    float track_x = viewport_w - (show_vbar ? thickness + SCROLLBAR_INSET : 0.0f);

    float thumb_h = thumb_length(track_y, content_h);
    float thumb_w = thumb_length(track_x, content_w);
    float travel_y = track_y - thumb_h;
    float travel_x = track_x - thumb_w;

    Clay_ElementId vthumb_id = { .id = ui_id_from_fmt("scroll_vthumb:%u", view->id) };
    Clay_ElementId hthumb_id = { .id = ui_id_from_fmt("scroll_hthumb:%u", view->id) };

    nk_point_t ptr = ui_pointer_location();

    /* Scrollbar drag — start */
    if (box.found && !scrollbar_drag.active && ui_pointer_press())
    {
        if (show_vbar && Clay_PointerOver(vthumb_id))
        {
            float thumb_top = (max_scroll_y > 0.0f) ? (scroll_y / max_scroll_y) * travel_y : 0.0f;
            scrollbar_drag.active = true;
            scrollbar_drag.id = view->id;
            scrollbar_drag.axis = SCROLL_AXIS_VERTICAL;
            scrollbar_drag.grab_offset = ptr.y - (box.boundingBox.y + thumb_top);
        }
        else if (show_hbar && Clay_PointerOver(hthumb_id))
        {
            float thumb_left = (max_scroll_x > 0.0f) ? (scroll_x / max_scroll_x) * travel_x : 0.0f;
            scrollbar_drag.active = true;
            scrollbar_drag.id = view->id;
            scrollbar_drag.axis = SCROLL_AXIS_HORIZONTAL;
            scrollbar_drag.grab_offset = ptr.x - (box.boundingBox.x + thumb_left);
        }
    }

    /* Scrollbar drag — track / release */
    if (scrollbar_drag.active && scrollbar_drag.id == view->id)
    {
        if (ui_pointer_down() && box.found && scroll_data.scrollPosition)
        {
            if (scrollbar_drag.axis == SCROLL_AXIS_VERTICAL && travel_y > 0.0f)
            {
                float thumb_top = ptr.y - scrollbar_drag.grab_offset - box.boundingBox.y;
                scroll_y = clamp01(thumb_top / travel_y) * max_scroll_y;
                scroll_data.scrollPosition->y = -scroll_y;
            }
            else if (scrollbar_drag.axis == SCROLL_AXIS_HORIZONTAL && travel_x > 0.0f)
            {
                float thumb_left = ptr.x - scrollbar_drag.grab_offset - box.boundingBox.x;
                scroll_x = clamp01(thumb_left / travel_x) * max_scroll_x;
                scroll_data.scrollPosition->x = -scroll_x;
            }
        }
        else if (!ui_pointer_down())
        {
            scrollbar_drag.active = false;
        }
    }

    /* Virtualisation window (vertical only) */
    size_t child_count = count_children(view);
    size_t first = 0;
    size_t last = child_count;

    float pitch = scroll_view->item_height + view->gap;

    if (scroll_view->virtualize && pitch > 0.0f)
    {
        long first_signed = (long)(scroll_y / pitch) - VIRTUAL_BUFFER;
        if (first_signed < 0) first_signed = 0;
        first = (size_t)first_signed;

        size_t visible = (size_t)(viewport_h / pitch) + 2 * VIRTUAL_BUFFER + 1;
        last = first + visible;
        if (last > child_count) last = child_count;
    }

    float top_spacer = (float)first * pitch;
    float bottom_spacer = (float)(child_count - last) * pitch;

    CLAY({
        .id = clay_id,
        .layout = {
            .sizing = {
                .width  = nk_to_clay_sizing(view->width),
                .height = nk_to_clay_sizing(view->height)
            },
            .padding = {
                .left   = view->padding.left,
                .top    = view->padding.top,
                .right  = view->padding.right,
                .bottom = view->padding.bottom
            },
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
        .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(view->background_resource)),
        .clip = {
            .horizontal  = true,
            .vertical    = true,
            .childOffset = Clay_GetScrollOffset()
        }
    })
    {
        /* Content column — fits its widest row so rows can overflow the viewport
           horizontally (driving the horizontal bar), while each row still grows
           to the column width so selection spans the full row. */
        CLAY({
            .layout = {
                .sizing = {
                    .width  = CLAY_SIZING_FIT(0),
                    .height = CLAY_SIZING_FIT(0)
                },
                .childGap = view->gap,
                .layoutDirection = CLAY_TOP_TO_BOTTOM
            }
        })
        {
            if (top_spacer > 0.0f)
            {
                CLAY({
                    .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(top_spacer) } }
                }) {}
            }

            nk_view_t *child = view->first_child;
            size_t index = 0;
            while (child)
            {
                if (index >= first && index < last)
                {
                    view_render(child);
                }
                child = child->next_sibling;
                index++;
            }

            if (bottom_spacer > 0.0f)
            {
                CLAY({
                    .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(bottom_spacer) } }
                }) {}
            }
        }
    }

    /* Scrollbar thumbs — floating, attached to the viewport so the clip's
       scroll offset does not move them and they stay inside the parent. */
    if (show_vbar)
    {
        float thumb_top = (max_scroll_y > 0.0f) ? (scroll_y / max_scroll_y) * travel_y : 0.0f;

        bool hot = (scrollbar_drag.active && scrollbar_drag.axis == SCROLL_AXIS_VERTICAL)
                 || Clay_PointerOver(vthumb_id);

        nk_color_t color = resource_get_dynamic_color(NKRES_COLOR_TEXT_SECONDARY);
        if (hot) color.a = 0.75f;

        CLAY({
            .id = vthumb_id,
            .floating = {
                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                .parentId = clay_id.id,
                .attachPoints = {
                    .element = CLAY_ATTACH_POINT_RIGHT_TOP,
                    .parent  = CLAY_ATTACH_POINT_RIGHT_TOP
                },
                .offset = { .x = -SCROLLBAR_INSET, .y = thumb_top },
                .zIndex = 5
            },
            .layout = {
                .sizing = {
                    .width  = CLAY_SIZING_FIXED(thickness),
                    .height = CLAY_SIZING_FIXED(thumb_h)
                }
            },
            .backgroundColor = nk_to_clay_color(color),
            .cornerRadius = CLAY_CORNER_RADIUS(thickness * 0.5f)
        }) {}
    }

    if (show_hbar)
    {
        float thumb_left = (max_scroll_x > 0.0f) ? (scroll_x / max_scroll_x) * travel_x : 0.0f;

        bool hot = (scrollbar_drag.active && scrollbar_drag.axis == SCROLL_AXIS_HORIZONTAL)
                 || Clay_PointerOver(hthumb_id);

        nk_color_t color = resource_get_dynamic_color(NKRES_COLOR_TEXT_SECONDARY);
        if (hot) color.a = 0.75f;

        CLAY({
            .id = hthumb_id,
            .floating = {
                .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
                .parentId = clay_id.id,
                .attachPoints = {
                    .element = CLAY_ATTACH_POINT_LEFT_BOTTOM,
                    .parent  = CLAY_ATTACH_POINT_LEFT_BOTTOM
                },
                .offset = { .x = thumb_left, .y = -SCROLLBAR_INSET },
                .zIndex = 5
            },
            .layout = {
                .sizing = {
                    .width  = CLAY_SIZING_FIXED(thumb_w),
                    .height = CLAY_SIZING_FIXED(thickness)
                }
            },
            .backgroundColor = nk_to_clay_color(color),
            .cornerRadius = CLAY_CORNER_RADIUS(thickness * 0.5f)
        }) {}
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

static size_t count_children(nk_view_t *view)
{
    size_t count = 0;
    nk_view_t *child = view->first_child;
    while (child)
    {
        count++;
        child = child->next_sibling;
    }
    return count;
}

static float thumb_length(float track, float content)
{
    if (content <= 0.0f) return SCROLLBAR_MIN_THUMB;

    float length = track * (track / content);

    if (length < SCROLLBAR_MIN_THUMB) length = SCROLLBAR_MIN_THUMB;
    if (length > track) length = track;

    return length;
}

static float clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}
