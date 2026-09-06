/***************************************************************
**
** Nanokit Source File
**
** File         :  table_row.c
** Module       :  ui/table
** Author       :  SH
** Created      :  2026-09-01 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Table Row Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

#include <ui/ui.h>

#include <resource/resource.h>

#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#define TABLE_ROW_HEIGHT  (24.0f)
#define TABLE_ROW_PADDING (8.0f)

/* U+2026. Swap for "..." if a font in use turns out to lack the glyph. */
#define TABLE_ROW_ELLIPSIS     "…"
#define TABLE_ROW_ELLIPSIS_LEN (sizeof(TABLE_ROW_ELLIPSIS) - 1)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static size_t fitting_prefix(const char *text, size_t length, float available,
    float size, nk_text_variant_t variant);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void nk_table_row_init(nk_table_row_t *row, nk_table_row_press_callback_t callback)
{
    row->view.type = NK_TABLE_ROW;
    row->view.id = ui_id_from_fmt("table_row:%p", row);

    row->text_info.color_resource = NKRES_COLOR_TEXT_PRIMARY;
    row->text_info.size = resource_get_float(NKRES_SIZE_TEXT_PRIMARY);

    row->view.width.type = NK_SIZING_GROW;
    row->view.height.type = NK_SIZING_FIXED;
    row->view.height.value = TABLE_ROW_HEIGHT;

    row->cell_count = 0;
    row->column_widths = NULL;

    row->selected = false;
    row->user_data = NULL;
    row->press_callback = callback;
}

void table_row_render(nk_view_t *view)
{
    nk_table_row_t *row = (nk_table_row_t *)view;

    if (!view->id) return;

    Clay_ElementId clay_id = { .id = row->view.id };

    bool hovered = Clay_PointerOver(clay_id);
    bool click = hovered && ui_pointer_press();

    if (click && row->press_callback)
    {
        row->press_callback(row);
    }

    nk_resource_t bg_res = row->selected ? NKRES_COLOR_BACKGROUND_SELECTED
                         : hovered       ? NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY
                         :                 NKRES_NONE;

    CLAY({
        .id = clay_id,
        .layout = {
            .sizing = {
                .width  = nk_to_clay_sizing(row->view.width),
                .height = nk_to_clay_sizing(row->view.height)
            },
            .childAlignment  = { .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_LEFT_TO_RIGHT
        },
        .backgroundColor = nk_to_clay_color(resource_get_dynamic_color(bg_res))
    })
    {
        for (size_t i = 0; i < row->cell_count; i++)
        {
            const char *text = row->cell_text[i] ? row->cell_text[i] : "";

            const float *width_ptr = row->column_widths ? row->column_widths[i] : NULL;
            float width = width_ptr ? *width_ptr : 0.0f;

            size_t length = strlen(text);

            /* Text longer than its column would otherwise run straight over the
               next one, so cut it where it stops fitting and mark the cut. The
               kept part is the original string with a shorter length — Clay
               carries a length alongside the pointer, so nothing has to be
               copied anywhere — and the mark is a literal after it. */
            size_t shown = length;

            if (width > 0.0f)
            {
                shown = fitting_prefix(text, length, width - 2.0f * TABLE_ROW_PADDING,
                    row->text_info.size, row->text_info.variant);
            }

            CLAY({
                .layout = {
                    .sizing = {
                        .width = (width > 0.0f)
                            ? CLAY_SIZING_FIXED(width)
                            : CLAY_SIZING_GROW(0)
                    },
                    .padding = {
                        .left  = (uint16_t)TABLE_ROW_PADDING,
                        .right = (uint16_t)TABLE_ROW_PADDING
                    },
                    .childAlignment = { .y = CLAY_ALIGN_Y_CENTER }
                }
            })
            {
                Clay_String label = { .chars = text, .length = (int32_t)shown };

                CLAY_TEXT(label, CLAY_TEXT_CONFIG({
                    .fontSize  = (uint16_t)row->text_info.size,
                    .textColor = nk_to_clay_color(resource_get_dynamic_color(row->text_info.color_resource)),
                    .fontId    = row->text_info.variant,
                    .wrapMode  = CLAY_TEXT_WRAP_NONE,
                }));

                if (shown < length)
                {
                    Clay_String ellipsis = {
                        .chars  = TABLE_ROW_ELLIPSIS,
                        .length = (int32_t)TABLE_ROW_ELLIPSIS_LEN
                    };

                    CLAY_TEXT(ellipsis, CLAY_TEXT_CONFIG({
                        .fontSize  = (uint16_t)row->text_info.size,
                        .textColor = nk_to_clay_color(resource_get_dynamic_color(row->text_info.color_resource)),
                        .fontId    = row->text_info.variant,
                        .wrapMode  = CLAY_TEXT_WRAP_NONE,
                    }));
                }
            }

            /* A dumb gap the same width as a header resize handle, so a
               column's body cells stay pixel-aligned with its header whether
               or not that particular column is resizable. */
            CLAY({
                .layout = {
                    .sizing = {
                        .width  = CLAY_SIZING_FIXED(resource_get_float(NKRES_SIZE_SPLITTER_THICKNESS)),
                        .height = CLAY_SIZING_GROW(0)
                    }
                }
            }) {}
        }

        /* Trailing filler, deliberately empty. Clay sizes a container to the
           sum of its children's *preferred* widths, and a text child prefers
           its full unwrapped width — so any cell that grows around text makes
           the row as wide as that row's longest string. Rows would then each
           demand a different width, the scroll view's content column would fit
           the widest of them, and every column past the growing one would sit
           at a different offset per row and different again in the header.
           Holding every cell to a fixed width and letting this text-free
           element — which prefers nothing — soak up the slack keeps every row
           exactly as wide as the header, whatever it contains. */
        CLAY({
            .layout = {
                .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) }
            }
        }) {}
    }
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

/* How much of `text` fits in `available`, leaving room for the ellipsis when
   the whole string does not. Returns `length` when it all fits. */
static size_t fitting_prefix(const char *text, size_t length, float available,
    float size, nk_text_variant_t variant)
{
    if (length == 0 || available <= 0.0f) return 0;

    if (ui_text_width(text, length, size, variant) <= available) return length;

    float room = available - ui_text_width(TABLE_ROW_ELLIPSIS, TABLE_ROW_ELLIPSIS_LEN, size, variant);
    if (room <= 0.0f) return 0;

    /* Largest prefix still inside `room`. Text only gets wider as it gets
       longer, so the fitting lengths are a prefix of the range and a bisection
       lands on the boundary in a handful of measurements. */
    size_t low = 0;
    size_t high = length;

    while (low < high)
    {
        size_t mid = low + (high - low + 1) / 2;

        if (ui_text_width(text, mid, size, variant) <= room) low = mid;
        else                                                 high = mid - 1;
    }

    /* Never cut inside a multi-byte character, or the tail renders as rubble.
       Continuation bytes are 10xxxxxx; step back off any that start the
       remainder. */
    while (low > 0 && ((unsigned char)text[low] & 0xC0) == 0x80) low--;

    return low;
}
