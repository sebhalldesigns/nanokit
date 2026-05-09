/***************************************************************
**
** Nanokit Source File
**
** File         :  gtk.c
** Module       :  backend/gtk
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit GTK Backend
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>
#include <ui/view/view.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <fontconfig/fontconfig.h>

#include <glad/glad.h>

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

    GtkWidget *gl_area;
    GtkGesture *gtk_click;
    GtkEventController *gtk_motion;
    GtkEventController *gtk_scroll;
    GtkEventController *gtk_hit_test;
} gtk_window_data_t;


/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static GtkApplication *app = NULL;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static void on_activate(GtkApplication *app, gpointer user_data);
static void on_realize(GtkGLArea *area, gpointer user_data);
static gboolean on_render(GtkGLArea *area, GdkGLContext *context, gpointer user_data);
static void on_resize(GtkGLArea *area, int width, int height, gpointer user_data);
static void on_motion(GtkEventControllerMotion *motion, double x, double y, gpointer user_data);
static void on_leave(GtkEventControllerMotion *motion, gpointer user_data);
static void on_pressed(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
static void on_released(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
static void on_click_cancel(GtkGesture *gesture, GdkEventSequence *seq, gpointer user_data);
static gboolean on_scroll(GtkEventControllerScroll *controller, double dx, double dy, gpointer user_data);

const char* find_font(const char *family);


/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool nk_window_create(nk_window_create_info_t *info, nk_window_t *window)
{
    GtkWidget *gtk_window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(gtk_window), info->start_width, info->start_height);
    gtk_window_set_title(GTK_WINDOW(gtk_window), info->title);

    gtk_window_data_t *data = calloc(1, sizeof(gtk_window_data_t));
    data->width = info->start_width;
    data->height = info->start_height;
    data->root_view = info->root;

    data->gl_area = gtk_gl_area_new();
    gtk_gl_area_set_required_version(GTK_GL_AREA(data->gl_area), 3, 3);
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(data->gl_area), true);

    gtk_widget_set_hexpand(data->gl_area, TRUE);
    gtk_widget_set_vexpand(data->gl_area, TRUE);

    g_signal_connect(data->gl_area, "realize", G_CALLBACK (on_realize), data);
    g_signal_connect(data->gl_area, "render", G_CALLBACK (on_render), data);

    g_signal_connect(data->gl_area, "resize", G_CALLBACK(on_resize), data);

    data->gtk_click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(data->gtk_click), 0); /* 0 = any button */
    g_signal_connect(data->gtk_click, "pressed",  G_CALLBACK(on_pressed),  data);
    g_signal_connect(data->gtk_click, "released", G_CALLBACK(on_released), data);
    g_signal_connect(data->gtk_click, "cancel",   G_CALLBACK(on_click_cancel), data);

    data->gtk_motion = gtk_event_controller_motion_new();
    g_signal_connect(data->gtk_motion, "motion", G_CALLBACK(on_motion), data);
    g_signal_connect(data->gtk_motion, "leave", G_CALLBACK(on_leave), data);

    data->gtk_scroll = gtk_event_controller_scroll_new(
            GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES
        |   GTK_EVENT_CONTROLLER_SCROLL_DISCRETE
    );
    g_signal_connect(data->gtk_scroll, "scroll", G_CALLBACK(on_scroll), data);

    gtk_widget_add_controller(data->gl_area, GTK_EVENT_CONTROLLER(data->gtk_click));
    gtk_widget_add_controller(data->gl_area, data->gtk_motion);
    gtk_widget_add_controller(data->gl_area, data->gtk_scroll);

    gtk_window_set_child (GTK_WINDOW(gtk_window), data->gl_area);

    gtk_window_present(GTK_WINDOW(gtk_window));

    return true;
}

int nk_run(nk_run_info_t *info, int argc, char **argv)
{
    printf("Hello world!\n");

    app = gtk_application_new(info->application_id, G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), info);

    return g_application_run(G_APPLICATION(app), argc, argv);
}


/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

static void on_activate(GtkApplication *app, gpointer user_data)
{
    printf("Activated\n");
    nk_run_info_t *info = (nk_run_info_t*)user_data;

    info->launch_callback();

}

static void on_realize(GtkGLArea *area, gpointer user_data)
{
    gtk_gl_area_make_current(area);

    gtk_window_data_t *data = (gtk_window_data_t*)user_data;

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

    if (!font_path || !view_context_init(&data->view_context, font_path))
    {
        fprintf(stderr, "Failed to initialize renderer.\n");
    }
}

static gboolean on_render(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
    gtk_window_data_t *data = (gtk_window_data_t*)user_data;

    //glViewport(0, 0, data->width, data->height);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view_context_render(&data->view_context, data->root_view, (float)data->width, (float)data->height, 1.0f,
                        (float)data->pointer_x, (float)data->pointer_y, data->pointer_down);

}

static void on_resize(GtkGLArea *area, int width, int height, gpointer user_data)
{
    gtk_window_data_t *data = (gtk_window_data_t*)user_data;
    data->width = width;
    data->height = height;

    gtk_gl_area_queue_render(GTK_GL_AREA(data->gl_area));

}

static void on_motion(GtkEventControllerMotion *motion, double x, double y, gpointer user_data)
{
    gtk_window_data_t *data = (gtk_window_data_t*)user_data;
    data->pointer_x = (float)x;
    data->pointer_y = (float)y;
    gtk_gl_area_queue_render(GTK_GL_AREA(data->gl_area));
}

static void on_leave(GtkEventControllerMotion *motion, gpointer user_data)
{
    gtk_window_data_t *data = (gtk_window_data_t*)user_data;
    data->pointer_x = (float)-1;
    data->pointer_y = (float)-1;
    gtk_gl_area_queue_render(GTK_GL_AREA(data->gl_area));
}

static void on_pressed(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));

    gtk_window_data_t *data = (gtk_window_data_t*)user_data;

    if (button == 1)
    {
        data->pointer_down = true;
    }

    gtk_gl_area_queue_render(GTK_GL_AREA(data->gl_area));
}

static void on_released(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));

    gtk_window_data_t *data = (gtk_window_data_t*)user_data;

    if (button == 1)
    {
        data->pointer_down = false;
    }

    gtk_gl_area_queue_render(GTK_GL_AREA(data->gl_area));
}

static void on_click_cancel(GtkGesture *gesture, GdkEventSequence *seq, gpointer user_data)
{
    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));

    gtk_window_data_t *data = (gtk_window_data_t*)user_data;

    if (button == 1)
    {
        data->pointer_down = false;
    }

    gtk_gl_area_queue_render(GTK_GL_AREA(data->gl_area));
}

static gboolean on_scroll(GtkEventControllerScroll *controller, double dx, double dy, gpointer user_data)
{
    gtk_window_data_t *data = (gtk_window_data_t*)user_data;
    gtk_gl_area_queue_render(GTK_GL_AREA(data->gl_area));
}

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
