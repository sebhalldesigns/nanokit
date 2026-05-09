/***************************************************************
**
** Nanokit Source File
**
** File         :  ui.c
** Module       :  ui
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit UI Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <ui/context/context.h>

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

static void emit_view(nk_view_t *view);
static void emit_children(nk_view_t *parent);
static Clay_Color nk_to_clay_color(nk_color_t c);
static Clay_SizingAxis nk_to_clay_sizing(nk_sizing_t s);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool ui_context_init(ui_context_t *context, const char* default_font)
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

    int font = nvgCreateFont(vg, "default", default_font);
    if (font == -1)
    {
        fprintf(stderr, "Failed to load font\n");
        return false;
    }

    g_vg = vg;

    Clay_SetMaxElementCount(65536);
    uint64_t totalSize = Clay_MinMemorySize();

    /* Initialize Clay */
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalSize, malloc(totalSize));

    Clay_Initialize(arena, (Clay_Dimensions){0, 0}, (Clay_ErrorHandler){0});
    Clay_SetMeasureTextFunction(measure_text, NULL);

    *context = (ui_context_t)vg;

    return true;
}

void ui_context_render(ui_context_t *context, nk_view_t *root, float width, float height, float dpr, float pointer_x, float pointer_y, bool mouse_down)
{
    NVGcontext* vg = (NVGcontext*)(*context);

    Clay_SetLayoutDimensions((Clay_Dimensions){width, height});
    Clay_SetPointerState((Clay_Vector2){.x = pointer_x, .y = pointer_y}, mouse_down);
    Clay_BeginLayout();

    emit_view(root);

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

static void emit_children(nk_view_t *parent)
{
    nk_view_t *child = parent->first_child;
    while (child)
    {
        emit_view(child);
        child = child->next_sibling;
    }
}

static Clay_Color nk_to_clay_color(nk_color_t c)
{
    return (Clay_Color){
        .r = c.r * 255.0f,
        .g = c.g * 255.0f,
        .b = c.b * 255.0f,
        .a = c.a * 255.0f
    };
}

static Clay_SizingAxis nk_to_clay_sizing(nk_sizing_t s)
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

static void emit_view(nk_view_t *view)
{
    switch (view->type)
    {

        case NK_LABEL:
        {
            nk_label_t *label = (nk_label_t *)view;
            Clay_String str = { .chars = label->text, .length = (int32_t)strlen(label->text) };
            CLAY_TEXT(str, CLAY_TEXT_CONFIG({
                .fontSize = (uint16_t)label->text_info.size,
                .textColor = nk_to_clay_color(label->text_info.color)
            }));
        }
        break;

        case NK_BUTTON:
        {
            nk_button_t *btn = (nk_button_t *)view;
            Clay_Color bg = nk_to_clay_color(view->background);

            if (view->id)
            {
                Clay_ElementId clay_id = Clay_GetElementId((Clay_String){
                    .chars = view->id,
                    .length = (int32_t)strlen(view->id)
                });

                if (Clay_PointerOver(clay_id))
                {
                    bg.r = (bg.r + 255) / 2;
                    bg.g = (bg.g + 255) / 2;
                    bg.b = (bg.b + 255) / 2;
                }
                
                Clay_String btn_str = { .chars = btn->text, .length = (int32_t)strlen(btn->text) };
                CLAY({
                    .id = clay_id,
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
                        }
                    },
                    .backgroundColor = bg,
                    .cornerRadius = CLAY_CORNER_RADIUS(view->corner_radius)
                }) {
                    CLAY_TEXT(btn_str, CLAY_TEXT_CONFIG({
                        .fontSize = (uint16_t)btn->text_info.size,
                        .textColor = nk_to_clay_color(btn->text_info.color)
                    }));
                }
            }
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
                    .childGap = view->gap,
                    .layoutDirection = (view->direction == NK_DIRECTION_HORIZONTAL)
                        ? CLAY_LEFT_TO_RIGHT
                        : CLAY_TOP_TO_BOTTOM
                },
                .backgroundColor = nk_to_clay_color(view->background),
                .cornerRadius = CLAY_CORNER_RADIUS(view->corner_radius)
            }) {
                emit_children(view);
            }
        }
        break;

    }
}
