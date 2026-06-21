/***************************************************************
**
** Nanokit Source File
**
** File         :  ui.c
** Module       :  ui/ui
** Author       :  SH
** Created      :  2026-05-16 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Base UI Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>
#define CLAY_IMPLEMENTATION
#include <clay/clay.h>

#include <ui/ui.h>

#include <resource/resource.h>
#include <backend/backend.h>

#include <glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#define FONS_USE_FREETYPE
#include <nanovg.h>
#include <nanovg_gl.h>

#include <string.h>

#include <stdio.h>
#include <stdarg.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static char id_buffer[ID_MAX_STRING_LENGTH];
static NVGcontext* g_vg = NULL;

static bool pointer_down_this_frame = false;
static bool pointer_down_last_frame = false;
static nk_point_t pointer_location;

static nk_run_info_t *run_info = NULL;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);
static void render_clay_commands(NVGcontext *vg);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void ui_set_info(nk_run_info_t *info)
{
    run_info = info;
}

nk_run_info_t *ui_get_info(void)
{
    return run_info;
}

bool ui_context_init(ui_context_t *context, ui_context_create_info_t *create_info)
{
    if (!gladLoadGL())
    {
        fprintf(stderr, "Failed to load OpenGL functions\n");
        return false;
    }

    NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    if (!vg)
    {
        fprintf(stderr, "Failed to initialize NanoVG context\n");
        return false;
    }

    int font = nvgCreateFont(vg, "default", create_info->default_font);
    if (font == -1)
    {
        fprintf(stderr, "Failed to load font\n");
        return false;
    }

    int bold_font = nvgCreateFont(vg, "default-bold", create_info->bold_font);
    if (bold_font == -1)
    {
        fprintf(stderr, "Failed to load bold font\n");
        return false;
    }

    g_vg = vg;

    Clay_SetMaxElementCount(65536);
    uint64_t totalSize = Clay_MinMemorySize();

    /* Initialize Clay */
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalSize, malloc(totalSize));

    Clay_Initialize(arena, (Clay_Dimensions){0, 0}, (Clay_ErrorHandler){0});
    Clay_SetMeasureTextFunction(measure_text, NULL);

    #if 0
    Clay_SetDebugModeEnabled(true);
    #endif

    *context = (ui_context_t)vg;

    return true;
}

void ui_context_render(ui_context_t *context, nk_view_t *root, ui_context_render_info_t *render_info)
{

    nk_window_set_cursor(backend_get_active_window(), NK_CURSOR_ARROW);

    NVGcontext* vg = (NVGcontext*)(*context);

    pointer_down_this_frame = render_info->mouse_down;
    pointer_location.x = render_info->pointer_x;
    pointer_location.y = render_info->pointer_y;

    Clay_SetLayoutDimensions((Clay_Dimensions){render_info->width, render_info->height});
    Clay_SetPointerState((Clay_Vector2){.x = render_info->pointer_x, .y = render_info->pointer_y - render_info->offset_y}, render_info->mouse_down);

    bool drag_scroll = false;

    #if __APPLE__
    drag_scroll = true;
    #endif

    static size_t last_time_micros = 0;
    size_t delta_time_micros = render_info->time_micros - last_time_micros;
    last_time_micros = render_info->time_micros;

    Clay_UpdateScrollContainers(drag_scroll, (Clay_Vector2){.x = render_info->scroll_delta_x, .y = render_info->scroll_delta_y}, (float)delta_time_micros * 0.000001f);


    Clay_BeginLayout();

    view_render(root);

    nvgBeginFrame(vg, render_info->width, render_info->height, render_info->dpr);

    //nvgTranslate(vg, 0, render_info->offset_y);

    nvgBeginPath(vg);

    #if _WIN32
    nvgRect(vg, 0.0f, 30.0f - render_info->offset_y, render_info->width, render_info->height - 30.0f + render_info->offset_y);
    #else
    nvgRect(vg, 0.0f, render_info->offset_y, render_info->width, render_info->height + render_info->offset_y);
    #endif

    nk_color_t background_secondary_color = resource_get_dynamic_color(NKRES_COLOR_BACKGROUND_TERTIARY);

    nvgFillColor(vg, nvgRGBA(
        background_secondary_color.r * 255.0f,
        background_secondary_color.g * 255.0f,
        background_secondary_color.b * 255.0f,
        background_secondary_color.a * 255.0f
    ));

    nvgFill(vg);

    render_clay_commands(vg);
    nvgEndFrame(vg);

    pointer_down_last_frame = pointer_down_this_frame;
}

uint32_t ui_id_from_fmt(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    vsnprintf(id_buffer, sizeof(id_buffer), message, args);
    va_end(args);

    Clay_ElementId clay_id = Clay_GetElementId((Clay_String){
        .chars = id_buffer,
        .length = (int32_t)strlen(id_buffer)
    });

    return clay_id.id;
}

Clay_Color nk_to_clay_color(nk_color_t c)
{
    return (Clay_Color){
        .r = c.r * 255.0f,
        .g = c.g * 255.0f,
        .b = c.b * 255.0f,
        .a = c.a * 255.0f
    };
}

Clay_SizingAxis nk_to_clay_sizing(nk_sizing_t s)
{
    switch (s.type)
    {
        case NK_SIZING_GROW:
            return CLAY_SIZING_GROW(.min = s.value, .max = s.max);
        case NK_SIZING_FIT:
            return CLAY_SIZING_FIT(.min = s.value, .max = s.max);
        case NK_SIZING_FIXED:
            return CLAY_SIZING_FIXED(s.value);
        case NK_SIZING_PERCENT:
            return CLAY_SIZING_PERCENT(s.value);
        default:
            return CLAY_SIZING_GROW(0);
    }
}


bool ui_pointer_press(void)
{
    return pointer_down_this_frame && !pointer_down_last_frame;
}

bool ui_pointer_release(void)
{
    return !pointer_down_this_frame && pointer_down_last_frame;
}

bool ui_pointer_down(void)
{
    return pointer_down_this_frame;
}

nk_point_t ui_pointer_location(void)
{
    return pointer_location;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/


static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData)
{
    (void)userData;

    nvgFontSize(g_vg, config->fontSize);

    switch (config->fontId)
    {
        case NK_TEXT_BOLD:
            nvgFontFace(g_vg, "default-bold");
            break;
        default:
            nvgFontFace(g_vg, "default");
            break;
    }

    float bounds[4];
    nvgTextBounds(g_vg, 0, 0, text.chars, text.chars + text.length, bounds);

    float ascender, descender, lineHeight;
    nvgTextMetrics(g_vg, &ascender, &descender, &lineHeight);

    return (Clay_Dimensions){
        .width = bounds[2] - bounds[0],
        .height = bounds[3] - bounds[1]
    };
}

static void render_clay_commands(NVGcontext *vg)
{
    Clay_RenderCommandArray commands = Clay_EndLayout();

    for (uint32_t i = 0; i < commands.length; i++)
    {
        Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(&commands, i);
        Clay_BoundingBox box = cmd->boundingBox;

        switch (cmd->commandType)
        {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
            {
                Clay_RectangleRenderData *data = &cmd->renderData.rectangle;
                /* Negative corner radii signal a concave outline (see BORDER); the
                   fill itself stays square there, so clamp negatives to 0. */
                float ftl = data->cornerRadius.topLeft     > 0 ? data->cornerRadius.topLeft     : 0.0f;
                float ftr = data->cornerRadius.topRight    > 0 ? data->cornerRadius.topRight    : 0.0f;
                float fbr = data->cornerRadius.bottomRight > 0 ? data->cornerRadius.bottomRight : 0.0f;
                float fbl = data->cornerRadius.bottomLeft  > 0 ? data->cornerRadius.bottomLeft  : 0.0f;
                nvgBeginPath(vg);
                nvgRoundedRectVarying(vg, box.x, box.y, box.width, box.height,
                                      ftl, ftr, fbr, fbl);
                nvgFillColor(vg, nvgRGBA(
                    data->backgroundColor.r,
                    data->backgroundColor.g,
                    data->backgroundColor.b,
                    data->backgroundColor.a
                ));
                nvgFill(vg);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_TEXT:
            {
                Clay_TextRenderData *data = &cmd->renderData.text;
                nvgFontSize(vg, data->fontSize);

                switch (data->fontId)
                {
                    case NK_TEXT_BOLD:
                        nvgFontFace(vg, "default-bold");
                        break;
                    default:
                        nvgFontFace(vg, "default");
                        break;
                }

                nvgFillColor(vg, nvgRGBA(
                    data->textColor.r,
                    data->textColor.g,
                    data->textColor.b,
                    data->textColor.a
                ));

                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
                nvgText(vg, box.x, box.y + box.height * 0.5f + 1.0f,
                            data->stringContents.chars,
                            data->stringContents.chars + data->stringContents.length);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_BORDER:
            {
                Clay_BorderRenderData *data = &cmd->renderData.border;

                float wl = data->width.left;
                float wr = data->width.right;
                float wt = data->width.top;
                float wb = data->width.bottom;
                if (wl <= 0 && wr <= 0 && wt <= 0 && wb <= 0) break;

                NVGcolor col = nvgRGBA(
                    data->color.r, data->color.g,
                    data->color.b, data->color.a
                );

                float rtl = data->cornerRadius.topLeft;
                float rtr = data->cornerRadius.topRight;
                float rbr = data->cornerRadius.bottomRight;
                float rbl = data->cornerRadius.bottomLeft;

                /* Fast path: the common case of a uniform, fully-closed border
                   (popups, tooltips, etc.) — a single rounded-rect stroke. */
                bool uniform_closed =
                    wl > 0 && wr > 0 && wt > 0 && wb > 0 &&
                    wl == wr && wt == wb && wl == wt &&
                    rtl >= 0 && rtr >= 0 && rbr >= 0 && rbl >= 0 &&
                    rtl == rtr && rtl == rbr && rtl == rbl;

                if (uniform_closed)
                {
                    nvgBeginPath(vg);
                    nvgRoundedRect(vg,
                        box.x + wt * 0.5f,
                        box.y + wt * 0.5f,
                        box.width  - wt,
                        box.height - wt,
                        rtl
                    );
                    nvgStrokeWidth(vg, wt);
                    nvgStrokeColor(vg, col);
                    nvgStroke(vg);
                    break;
                }


                /* General path: honours per-side widths and per-corner radii.
                   A negative radius draws a *concave* fillet that flares the
                   corner outward (used for the bottom of dock tabs so the tab
                   outline blends into the tab-bar underside). */
                float sw = wl;
                if (wr > sw) sw = wr;
                if (wt > sw) sw = wt;
                if (wb > sw) sw = wb;

                float in = sw * 0.5f;
                float L  = box.x + in;
                float T  = box.y + in;
                float R  = box.x + box.width  - in;
                float Bm = box.y + box.height - in;

                float aTL = rtl < 0 ? -rtl : rtl;
                float aTR = rtr < 0 ? -rtr : rtr;
                float aBR = rbr < 0 ? -rbr : rbr;
                float aBL = rbl < 0 ? -rbl : rbl;

                const float PI_ = NVG_PI;

                nvgStrokeColor(vg, col);
                nvgStrokeWidth(vg, sw);
                nvgLineCap(vg, NVG_BUTT);
                nvgLineJoin(vg, NVG_ROUND);
                nvgBeginPath(vg);

                /* Walk the perimeter clockwise. Each present segment continues
                   the current sub-path if its start meets the pen, otherwise a
                   new sub-path is started. Sides with width 0 (and corners with
                   radius 0) are skipped, which naturally leaves gaps (e.g. the
                   open bottom of a tab). */
                bool  has_pen = false;
                float cx = 0.0f, cy = 0.0f;

                #define NK_PEN_AT(px, py) \
                    (has_pen && fabsf((px) - cx) < 0.01f && fabsf((py) - cy) < 0.01f)

                /* Top side */
                if (wt > 0)
                {
                    float sx = L + aTL, sy = T;
                    float ex = R - aTR, ey = T;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    nvgLineTo(vg, ex, ey);
                    cx = ex; cy = ey; has_pen = true;
                }
                /* Top-right corner */
                if (aTR > 0)
                {
                    float sx = R - aTR, sy = T;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    if (rtr >= 0)
                        nvgArc(vg, R - aTR, T + aTR, aTR, 1.5f * PI_, 2.0f * PI_, NVG_CW);
                    else
                        nvgArc(vg, R + aTR, T + aTR, aTR, PI_, 1.5f * PI_, NVG_CW);
                    cx = R; cy = T + aTR; has_pen = true;
                }
                /* Right side */
                if (wr > 0)
                {
                    float sx = R, sy = T + aTR;
                    float ex = R, ey = Bm - aBR;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    nvgLineTo(vg, ex, ey);
                    cx = ex; cy = ey; has_pen = true;
                }
                /* Bottom-right corner */
                if (aBR > 0)
                {
                    float sx = R, sy = Bm - aBR;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    if (rbr >= 0)
                        nvgArc(vg, R - aBR, Bm - aBR, aBR, 0.0f, 0.5f * PI_, NVG_CW);
                    else
                        nvgArc(vg, R + aBR, Bm - aBR, aBR, PI_, 0.5f * PI_, NVG_CCW);
                    cx = (rbr >= 0) ? (R - aBR) : (R + aBR); cy = Bm; has_pen = true;
                }
                /* Bottom side */
                if (wb > 0)
                {
                    float sx = R - aBR, sy = Bm;
                    float ex = L + aBL, ey = Bm;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    nvgLineTo(vg, ex, ey);
                    cx = ex; cy = ey; has_pen = true;
                }
                /* Bottom-left corner */
                if (aBL > 0)
                {
                    float sx = (rbl >= 0) ? (L + aBL) : (L - aBL), sy = Bm;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    if (rbl >= 0)
                        nvgArc(vg, L + aBL, Bm - aBL, aBL, 0.5f * PI_, PI_, NVG_CW);
                    else
                        nvgArc(vg, L - aBL, Bm - aBL, aBL, 0.5f * PI_, 0.0f, NVG_CCW);
                    cx = L; cy = Bm - aBL; has_pen = true;
                }
                /* Left side */
                if (wl > 0)
                {
                    float sx = L, sy = Bm - aBL;
                    float ex = L, ey = T + aTL;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    nvgLineTo(vg, ex, ey);
                    cx = ex; cy = ey; has_pen = true;
                }
                /* Top-left corner */
                if (aTL > 0)
                {
                    float sx = L, sy = T + aTL;
                    if (!NK_PEN_AT(sx, sy)) nvgMoveTo(vg, sx, sy);
                    if (rtl >= 0)
                        nvgArc(vg, L + aTL, T + aTL, aTL, PI_, 1.5f * PI_, NVG_CW);
                    else
                        nvgArc(vg, L - aTL, T + aTL, aTL, 0.0f, 0.5f * PI_, NVG_CCW);
                    cx = L + aTL; cy = T; has_pen = true;
                }

                #undef NK_PEN_AT

                nvgStroke(vg);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
            {
                nvgScissor(vg, box.x, box.y, box.width, box.height);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
            {
                nvgResetScissor(vg);
                break;
            }

            default:
                break;
        }
    }
}
