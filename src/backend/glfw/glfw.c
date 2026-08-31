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
#include <ui/ui.h>


#include <fontconfig/fontconfig.h>

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <gio/gio.h>

#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
    ui_context_t view_context;
    nk_view_t *root_view;

} glfw_window_data_t;


/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static bool running = false;
static GLFWwindow *single_window = NULL;

static GLFWcursor *arrow_cusor = NULL;
static GLFWcursor *ibeam_cusor = NULL;
static GLFWcursor *hand_cusor = NULL;
static GLFWcursor *resize_ns_cusor = NULL;
static GLFWcursor *resize_ew_cusor = NULL;

static bool open_file_queued = false;
static bool single_file = false;
static nk_file_callback_t open_file_callback = NULL;

static bool open_directory_queued = false;
static nk_directory_callback_t open_directory_callback = NULL;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

const char* find_font(const char *family);

static void error_callback(int error, const char* description);
static void on_framebuffer_size(GLFWwindow *w, int width, int height);
static void on_cursor_pos(GLFWwindow *w, double x, double y);
static void on_mouse_button(GLFWwindow *w, int button, int action, int mods);
static void on_scroll(GLFWwindow *w, double dx, double dy);

static void open_files(bool single, nk_file_callback_t callback);
static void open_directory(nk_directory_callback_t callback);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool backend_init()
{
    if (!glfwInit())
    {
        return false;
    }

    arrow_cusor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    ibeam_cusor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    hand_cusor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    resize_ns_cusor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    resize_ew_cusor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

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

    ui_context_create_info_t view_create_info = {
        .default_font = font_path,
        .bold_font = font_path
    };

    glfw_window_data_t *data = calloc(1, sizeof(glfw_window_data_t));
    data->width = info->start_width;
    data->height = info->start_height;
    data->root_view = info->root;


    if (!font_path || !ui_context_init(&data->view_context, &view_create_info))
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

    glfwWindowHintString(GLFW_WAYLAND_APP_ID, info->application_id);

    info->launch_callback();

    running = true;

    while (!glfwWindowShouldClose(single_window))
    {
        glfwWaitEvents();

        if (open_file_queued)
        {
            open_file_queued = false;
            open_files(single_file, open_file_callback);
        }

        if (open_directory_queued)
        {
            open_directory_queued = false;
            open_directory(open_directory_callback);
        }

        glfw_window_data_t *data = (glfw_window_data_t*)glfwGetWindowUserPointer(single_window);

        glfwMakeContextCurrent(single_window);

        glViewport(0, 0, data->width, data->height);

        glDisable(GL_SCISSOR_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        /* Clear the stencil too — NanoVG's NVG_STENCIL_STROKES path draws each
           stroke base only where stencil == 0, so stale/garbage stencil (e.g.
           freshly reallocated regions after a resize) masks out parts of border
           strokes and they render dashed. */
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ui_context_render_info_t render_info = {
            .width = (float)data->width,
            .height = (float)data->height,
            .offset_y = 0.0f,
            .dpr = 1.0f,
            .dark_mode = true,
            .pointer_x = (float)data->pointer_x,
            .pointer_y = (float)data->pointer_y,
            .mouse_down = data->pointer_down
        };

        ui_context_render(&data->view_context, data->root_view, &render_info);

        glfwSwapBuffers(single_window);

    }

    glfwDestroyCursor(arrow_cusor);
    glfwDestroyCursor(ibeam_cusor);
    glfwDestroyCursor(hand_cusor);
    glfwDestroyCursor(resize_ns_cusor);
    glfwDestroyCursor(resize_ew_cusor);

    glfwTerminate();

    return 0;
}

void nk_window_set_cursor(nk_window_t *window, nk_cursor_t cursor)
{
    GLFWcursor *glfw_cursor;

    switch (cursor)
    {
        case NK_CURSOR_IBEAM:
        {
            glfw_cursor = ibeam_cusor;
        } break;

        case NK_CURSOR_HAND:
        {
            glfw_cursor = hand_cusor;
        } break;

        case NK_CURSOR_RESIZE_NS:
        {
            glfw_cursor = resize_ns_cusor;
        } break;

        case NK_CURSOR_RESIZE_EW:
        {
            glfw_cursor = resize_ew_cusor;
        } break;

        default:
        {
            glfw_cursor = arrow_cusor;
        } break;
    }

    glfwSetCursor((GLFWwindow*)window, glfw_cursor);
}

nk_window_t *backend_get_active_window(void)
{
    return (nk_window_t*)single_window;
}

void nk_window_request_redraw(nk_window_t *window)
{
    /* wake glfwWaitEvents() so the run loop redraws promptly */
    glfwPostEmptyEvent();
}

void nk_io_open_files(bool single_file_param, nk_file_callback_t callback)
{
    assert(callback);

    open_file_callback = callback;
    single_file = single_file_param;
    open_file_queued = true;
}

void nk_io_open_directory(nk_directory_callback_t callback)
{
    assert(callback);

    open_directory_callback = callback;
    open_directory_queued = true;
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


/***************************************************************
** MARK: FILE DIALOGS (XDG DESKTOP PORTAL)
***************************************************************/

/* GLFW's Wayland backend loads libdecor, whose default plugin links GTK 3.
   Linking a GUI toolkit of our own into the process would put a second GTK
   major in the same GObject type system, so the dialogs talk to the
   xdg-desktop-portal over D-Bus instead. That needs GIO only, and gives the
   host desktop's native file chooser on GNOME, KDE and everything else with a
   portal implementation. */

#define PORTAL_BUS_NAME   "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT     "/org/freedesktop/portal/desktop"
#define PORTAL_FILECHOOSER "org.freedesktop.portal.FileChooser"
#define PORTAL_REQUEST    "org.freedesktop.portal.Request"

typedef struct {
    bool done;
    char **paths;
    size_t count;
} portal_request_t;

static void on_portal_response(
    GDBusConnection *connection,
    const char *sender_name,
    const char *object_path,
    const char *interface_name,
    const char *signal_name,
    GVariant *parameters,
    gpointer user_data)
{
    portal_request_t *request = (portal_request_t*)user_data;

    guint32 response = 1;
    GVariant *results = NULL;

    g_variant_get(parameters, "(u@a{sv})", &response, &results);

    if (response == 0 && results)
    {
        GVariant *uris = g_variant_lookup_value(
            results, "uris", G_VARIANT_TYPE_STRING_ARRAY);

        if (uris)
        {
            gsize n = g_variant_n_children(uris);

            request->paths = (char**)calloc(n, sizeof(char*));

            for (gsize i = 0; i < n; i++)
            {
                const char *uri = NULL;
                g_variant_get_child(uris, i, "&s", &uri);

                /* portal results are URIs; nanokit's API deals in paths */
                char *path = g_filename_from_uri(uri, NULL, NULL);

                if (path)
                {
                    request->paths[request->count++] = path;
                }
            }

            g_variant_unref(uris);
        }
    }

    if (results)
    {
        g_variant_unref(results);
    }

    request->done = true;
}

/* The portal answers on a request object whose path we can derive up front, so
   we subscribe before making the call and cannot miss the response. */
static char *portal_request_path(GDBusConnection *connection, const char *token)
{
    const char *unique_name = g_dbus_connection_get_unique_name(connection);

    if (!unique_name)
    {
        return NULL;
    }

    /* ":1.42" -> "1_42" */
    char *sender = g_strdup(unique_name + 1);

    for (char *c = sender; *c; c++)
    {
        if (*c == '.')
        {
            *c = '_';
        }
    }

    char *path = g_strdup_printf(
        "%s/request/%s/%s", PORTAL_OBJECT, sender, token);

    g_free(sender);

    return path;
}

/* Runs the portal's OpenFile request to completion. Returns false if the
   portal could not be reached at all; otherwise fills in `request` (a count of
   zero means the user cancelled). */
static bool portal_open(const char *title, bool multiple, bool directory,
                        portal_request_t *request)
{
    GError *error = NULL;

    GDBusConnection *connection =
        g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);

    if (!connection)
    {
        fprintf(stderr, "nanokit: no session bus: %s\n",
                error ? error->message : "unknown error");
        g_clear_error(&error);
        return false;
    }

    static unsigned int token_counter = 0;
    char *token = g_strdup_printf("nanokit%u", ++token_counter);

    char *request_path = portal_request_path(connection, token);

    if (!request_path)
    {
        g_free(token);
        g_object_unref(connection);
        return false;
    }

    guint subscription = g_dbus_connection_signal_subscribe(
        connection,
        PORTAL_BUS_NAME,
        PORTAL_REQUEST,
        "Response",
        request_path,
        NULL,
        G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE,
        on_portal_response,
        request,
        NULL);

    GVariantBuilder options;
    g_variant_builder_init(&options, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&options, "{sv}", "handle_token",
                          g_variant_new_string(token));
    g_variant_builder_add(&options, "{sv}", "multiple",
                          g_variant_new_boolean(multiple));
    g_variant_builder_add(&options, "{sv}", "directory",
                          g_variant_new_boolean(directory));

    GVariant *reply = g_dbus_connection_call_sync(
        connection,
        PORTAL_BUS_NAME,
        PORTAL_OBJECT,
        PORTAL_FILECHOOSER,
        "OpenFile",
        g_variant_new("(ssa{sv})", "", title, &options),
        G_VARIANT_TYPE("(o)"),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error);

    if (!reply)
    {
        fprintf(stderr, "nanokit: file dialog failed: %s\n",
                error ? error->message : "unknown error");
        g_clear_error(&error);
        g_dbus_connection_signal_unsubscribe(connection, subscription);
        g_free(request_path);
        g_free(token);
        g_object_unref(connection);
        return false;
    }

    /* The portal is free to pick a different path than the one we derived; if
       it did, resubscribe on the path it actually returned. */
    const char *actual_path = NULL;
    g_variant_get(reply, "(&o)", &actual_path);

    if (g_strcmp0(actual_path, request_path) != 0)
    {
        g_dbus_connection_signal_unsubscribe(connection, subscription);

        subscription = g_dbus_connection_signal_subscribe(
            connection,
            PORTAL_BUS_NAME,
            PORTAL_REQUEST,
            "Response",
            actual_path,
            NULL,
            G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE,
            on_portal_response,
            request,
            NULL);
    }

    g_variant_unref(reply);

    while (!request->done)
    {
        g_main_context_iteration(NULL, TRUE);
    }

    g_dbus_connection_signal_unsubscribe(connection, subscription);
    g_free(request_path);
    g_free(token);
    g_object_unref(connection);

    return true;
}

static void portal_request_free(portal_request_t *request)
{
    for (size_t i = 0; i < request->count; i++)
    {
        g_free(request->paths[i]);
    }

    free(request->paths);
}

static void open_files(bool single, nk_file_callback_t callback)
{
    assert(callback);

    portal_request_t request = { 0 };

    if (!portal_open("Open File", !single, false, &request))
    {
        callback(false, NULL, 0);
        return;
    }

    if (request.count == 0)
    {
        callback(false, NULL, 0);
    }
    else
    {
        callback(true, (const char**)request.paths, request.count);
    }

    portal_request_free(&request);
}

static void open_directory(nk_directory_callback_t callback)
{
    assert(callback);

    portal_request_t request = { 0 };

    if (!portal_open("Open Folder", false, true, &request))
    {
        callback(false, NULL);
        return;
    }

    if (request.count == 0)
    {
        callback(false, NULL);
    }
    else
    {
        callback(true, request.paths[0]);
    }

    portal_request_free(&request);
}
