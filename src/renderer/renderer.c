/***************************************************************
**
** Nanokit Source File
**
** File         :  renderer.c
** Module       :  renderer
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Renderer Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <renderer/renderer.h>

#include <glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#define FONS_USE_FREETYPE
#include <nanovg.h>
#include <nanovg_gl.h>

#define CLAY_IMPLEMENTATION
#include <clay/clay.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/
static NVGcontext* g_vg = NULL;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);
static void render_clay_commands(NVGcontext *vg);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool renderer_init(render_context_t *context)
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

    int font = nvgCreateFont(vg, "default", "C:/Windows/Fonts/segoeui.ttf");
    if (font == -1)
    {
        fprintf(stderr, "Failed to load font\n");
        return false;
    }

    g_vg = vg;

    /* Initialize Clay */
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(
        Clay_MinMemorySize(), malloc(Clay_MinMemorySize())
    );

    Clay_Initialize(arena, (Clay_Dimensions){0, 0}, (Clay_ErrorHandler){0});
    Clay_SetMeasureTextFunction(measure_text, NULL);

    *context = (render_context_t)vg;

    return true;
}

void renderer_render(render_context_t *context, float width, float height, float dpr)
{
    NVGcontext* vg = (NVGcontext*)(*context);

    Clay_SetLayoutDimensions((Clay_Dimensions){width, height});
    Clay_BeginLayout();

    CLAY({
        .id = CLAY_ID("Root"),
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(0),
                .height = CLAY_SIZING_GROW(0)
            },
            .padding = CLAY_PADDING_ALL(16),
            .childGap = 8,
            .layoutDirection = CLAY_TOP_TO_BOTTOM
        },
        .backgroundColor = {45, 45, 45, 255}
    }) {
        CLAY_TEXT(CLAY_STRING("BusLab"), CLAY_TEXT_CONFIG({
            .fontSize = 20,
            .textColor = {220, 220, 220, 255}
        }));

        CLAY({
            .id = CLAY_ID("ContentArea"),
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_GROW(0),
                    .height = CLAY_SIZING_GROW(0)
                },
                .padding = CLAY_PADDING_ALL(12)
            },
            .backgroundColor = {35, 35, 35, 255},
            .cornerRadius = CLAY_CORNER_RADIUS(4)
        }) {
            CLAY_TEXT(CLAY_STRING("CAN Signal Viewer"), CLAY_TEXT_CONFIG({
                .fontSize = 14,
                .textColor = {180, 180, 180, 255}
            }));
        }
    }

    nvgBeginFrame(vg, width, height, dpr);
    render_clay_commands(vg);
    nvgEndFrame(vg);
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData)
{
    (void)userData;

    nvgFontSize(g_vg, config->fontSize);
    nvgFontFace(g_vg, "default");

    float bounds[4];
    nvgTextBounds(g_vg, 0, 0, text.chars, text.chars + text.length, bounds);

    float ascender, descender, lineHeight;
    nvgTextMetrics(g_vg, &ascender, &descender, &lineHeight);

    return (Clay_Dimensions){
        .width = bounds[2] - bounds[0],
        .height = lineHeight
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
                nvgFontFace(vg, "default");
                nvgFillColor(vg, nvgRGBA(
                    data->textColor.r,
                    data->textColor.g,
                    data->textColor.b,
                    data->textColor.a
                ));
                nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgText(vg, box.x, box.y,
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

                if (data->width.top > 0)
                {
                    nvgBeginPath(vg);
                    nvgStrokeWidth(vg, data->width.top);
                    nvgMoveTo(vg, box.x, box.y);
                    nvgLineTo(vg, box.x + box.width, box.y);
                    nvgStroke(vg);
                }
                if (data->width.bottom > 0)
                {
                    nvgBeginPath(vg);
                    nvgStrokeWidth(vg, data->width.bottom);
                    nvgMoveTo(vg, box.x, box.y + box.height);
                    nvgLineTo(vg, box.x + box.width, box.y + box.height);
                    nvgStroke(vg);
                }
                if (data->width.left > 0)
                {
                    nvgBeginPath(vg);
                    nvgStrokeWidth(vg, data->width.left);
                    nvgMoveTo(vg, box.x, box.y);
                    nvgLineTo(vg, box.x, box.y + box.height);
                    nvgStroke(vg);
                }
                if (data->width.right > 0)
                {
                    nvgBeginPath(vg);
                    nvgStrokeWidth(vg, data->width.right);
                    nvgMoveTo(vg, box.x + box.width, box.y);
                    nvgLineTo(vg, box.x + box.width, box.y + box.height);
                    nvgStroke(vg);
                }
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
