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
#define CLAY_IMPLEMENTATION
#include <clay/clay.h>

#include <resource/resource.h>
#include <ui/view/view.h>

#include <glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#define FONS_USE_FREETYPE
#include <nanovg.h>
#include <nanovg_gl.h>

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

static NVGcontext* g_vg = NULL;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static Clay_Dimensions measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);
static void render_clay_commands(NVGcontext *vg);

static void render_view(nk_view_t *view);
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

bool view_context_init(view_context_t *context, view_context_create_info_t *create_info)
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

    *context = (view_context_t)vg;

    return true;
}

void view_context_render(view_context_t *context, nk_view_t *root, view_context_render_info_t *render_info)
{
    NVGcontext* vg = (NVGcontext*)(*context);

    Clay_SetLayoutDimensions((Clay_Dimensions){render_info->width, render_info->height});
    Clay_SetPointerState((Clay_Vector2){.x = render_info->pointer_x, .y = render_info->pointer_y - render_info->offset_y}, render_info->mouse_down);
    Clay_BeginLayout();

    render_view(root);

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

static void render_view(nk_view_t *view)
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
                render_children(view);
            }
        }
        break;

    }
}


static void render_children(nk_view_t *parent)
{
    nk_view_t *child = parent->first_child;
    while (child)
    {
        render_view(child);
        child = child->next_sibling;
    }
}
