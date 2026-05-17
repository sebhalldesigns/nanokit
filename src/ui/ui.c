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

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);
static void render_clay_commands(NVGcontext *vg);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

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
    NVGcontext* vg = (NVGcontext*)(*context);

    pointer_down_this_frame = render_info->mouse_down;
    pointer_location.x = render_info->pointer_x;
    pointer_location.y = render_info->pointer_y;

    Clay_SetLayoutDimensions((Clay_Dimensions){render_info->width, render_info->height});
    Clay_SetPointerState((Clay_Vector2){.x = render_info->pointer_x, .y = render_info->pointer_y - render_info->offset_y}, render_info->mouse_down);
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
                nvgBeginPath(vg);
                nvgRoundedRect(vg, box.x, box.y, box.width, box.height,
                               data->cornerRadius.topLeft);
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

                /* Draw each border side that has width > 0 */
                nvgStrokeColor(vg, nvgRGBA(
                    data->color.r, data->color.g,
                    data->color.b, data->color.a
                ));

                float width = data->width.top;
                if (width <= 0) break;

                nvgBeginPath(vg);
                nvgRoundedRect(vg,
                    box.x + width * 0.5f,
                    box.y + width * 0.5f,
                    box.width  - width,
                    box.height - width,
                    data->cornerRadius.topLeft
                );
                nvgStrokeWidth(vg, width);
                nvgStrokeColor(vg, nvgRGBA(
                    data->color.r, data->color.g,
                    data->color.b, data->color.a
                ));
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
