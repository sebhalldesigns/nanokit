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

    *context = (render_context_t)vg;

    return true;
}

void renderer_render(render_context_t *context, float width, float height, float dpr)
{
    NVGcontext* vg = (NVGcontext*)(*context);

    nvgBeginFrame(vg, width, height, dpr);

    nvgFillColor(vg, nvgRGBA(255, 0, 0, 255));
    nvgBeginPath(vg);
    nvgRect(vg, 100, 100, 200, 200);
    nvgFill(vg);

    nvgFontSize(vg, 12.0f);
    nvgFontFace(vg, "default");
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
    nvgText(vg, 10, 10, "BusLab", NULL);

    nvgEndFrame(vg);
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
