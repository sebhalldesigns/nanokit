/***************************************************************
**
** Nanokit Source File
**
** File         :  glfw.c
** Module       :  backend/glfw
** Author       :  SH
** Created      :  2026-05-13 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit GLFW Backend
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "backend/backend.h"

#include <locale.h>
#include <resource/resource.h>

#include <nanokit.h>
#include <ui/view/view.h>


#include <fontconfig/fontconfig.h>

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/


/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef struct {

    int width;
    int height;
    int pointer_x;
    int pointer_y;
    bool pointer_down;
    view_context_t view_context;
    nk_view_t *root_view;

} glfw_window_data_t;


/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static bool running = false;
static GLFWwindow *single_window = NULL;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

const char* find_font(const char *family);

static void error_callback(int error, const char* description);
static void on_framebuffer_size(GLFWwindow *w, int width, int height);
static void on_cursor_pos(GLFWwindow *w, double x, double y);
static void on_mouse_button(GLFWwindow *w, int button, int action, int mods);
static void on_scroll(GLFWwindow *w, double dx, double dy);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool backend_init()
{
    if (!glfwInit())
    {
        return false;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    return true;
}

bool nk_window_create(nk_window_create_info_t *info, nk_window_t *window)
{

    GLFWwindow* glfw_window = glfwCreateWindow(
        info->start_width,
        info->start_height,
        info->title,
        NULL, NULL
    );

    if (!glfw_window)
    {
        return false;
    }

    glfwShowWindow(glfw_window);

    glfwMakeContextCurrent(glfw_window);
    glfwSwapInterval(1);

    const char *families[] = {
        "DejaVu Sans",
        "Liberation Sans",
        "Noto Sans",
        NULL
    };

    const char *font_path = NULL;
    for (int i = 0; families[i]; i++) {
        font_path = find_font(families[i]);
        if (font_path) break;
    }

    view_context_create_info_t view_create_info = {
        .default_font = font_path,
        .bold_font = font_path
    };

    glfw_window_data_t *data = calloc(1, sizeof(glfw_window_data_t));
    data->width = info->start_width;
    data->height = info->start_height;
    data->root_view = info->root;


    if (!font_path || !view_context_init(&data->view_context, &view_create_info))
    {
        fprintf(stderr, "Failed to initialize renderer.\n");
    }

    resource_set_system_appearance(NK_RESOURCE_APPEARANCE_DARK);

    glfwSetWindowUserPointer(glfw_window, data);

    glfwSetFramebufferSizeCallback(glfw_window, on_framebuffer_size);
    glfwSetCursorPosCallback(glfw_window, on_cursor_pos);
    glfwSetMouseButtonCallback(glfw_window, on_mouse_button);
    glfwSetScrollCallback(glfw_window, on_scroll);

    *window = (nk_window_t)glfw_window;

    single_window = glfw_window;

    return true;
}

int backend_run(nk_run_info_t *info, int argc, char **argv)
{

    info->launch_callback();

    running = true;

    while (running)
    {
        glfwWaitEvents();

        glfw_window_data_t *data = (glfw_window_data_t*)glfwGetWindowUserPointer(single_window);

        glfwMakeContextCurrent(single_window);

        glViewport(0, 0, data->width, data->height);

        glDisable(GL_SCISSOR_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view_context_render_info_t render_info = {
            .width = (float)data->width,
            .height = (float)data->height,
            .offset_y = 0.0f,
            .dpr = 1.0f,
            .dark_mode = true,
            .pointer_x = (float)data->pointer_x,
            .pointer_y = (float)data->pointer_y,
            .mouse_down = data->pointer_down
        };

        view_context_render(&data->view_context, data->root_view, &render_info);

        glfwSwapBuffers(single_window);


    }

    return 0;
}


/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/


const char* find_font(const char *family)
{
    FcConfig *config = FcInitLoadConfigAndFonts();
    FcPattern *pat = FcNameParse((const FcChar8*)family);
    FcConfigSubstitute(config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcResult result;
    FcPattern *font = FcFontMatch(config, pat, &result);

    FcChar8 *path = NULL;
    if (font)
        FcPatternGetString(font, FC_FILE, 0, &path);

    return path ? strdup((const char*)path) : NULL;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void on_framebuffer_size(GLFWwindow *w, int width, int height)
{
    glfw_window_data_t *data = glfwGetWindowUserPointer(w);
    data->width = width;
    data->height = height;
}

static void on_cursor_pos(GLFWwindow *w, double x, double y)
{
    glfw_window_data_t *data = glfwGetWindowUserPointer(w);
    data->pointer_x = (int)x;
    data->pointer_y = (int)y;
}

static void on_mouse_button(GLFWwindow *w, int button, int action, int mods)
{
    glfw_window_data_t *data = glfwGetWindowUserPointer(w);
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        data->pointer_down = (action == GLFW_PRESS);
}

static void on_scroll(GLFWwindow *w, double dx, double dy)
{
    /* todo */
}
