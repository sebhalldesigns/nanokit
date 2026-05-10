/***************************************************************
**
** Nanokit Header File
**
** File         :  nanokit.h
** Module       :  nanokit
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Interface Definition
**
***************************************************************/

#ifndef NANOKIT_H
#define NANOKIT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <windowsx.h>
#endif

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#ifndef NDEBUG
/* Production builds should set NDEBUG=1 */
#define NDEBUG false
#else
#define NDEBUG true
#endif

#ifndef DEBUG
#define DEBUG !NDEBUG
#endif


/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef void (*nk_void_callback_t)(void);
typedef bool (*nk_bool_callback_t)(void);

struct nk_view_t;

typedef struct
{
    const char *application_id;
    nk_void_callback_t launch_callback;
} nk_run_info_t;

typedef uintptr_t nk_window_t;

typedef struct
{
    const char *title;
    int start_width;
    int start_height;
    int min_width;
    int min_height;
    struct nk_view_t *root;
} nk_window_create_info_t;

typedef struct {
    float r, g, b, a;
} nk_color_t;

typedef struct {
    float left, top, right, bottom;
} nk_thickness_t;

typedef enum {
    NK_DIRECTION_VERTICAL,
    NK_DIRECTION_HORIZONTAL
} nk_direction_t;

typedef enum {
    NK_SIZING_GROW,
    NK_SIZING_FIT,
    NK_SIZING_FIXED,
    NK_SIZING_PERCENT
} nk_sizing_type_t;

typedef struct {
    nk_sizing_type_t type;
    float value;     /* fixed: pixels, percent: 0-1, grow/fit: min */
    float max;       /* optional max for grow/fit */
} nk_sizing_t;

typedef enum {
    NK_ALIGN_START,
    NK_ALIGN_CENTER,
    NK_ALIGN_END
} nk_align_t;

typedef struct nk_view_t
{
    size_t type;
    const char *id;

    nk_color_t background;
    float corner_radius;

    nk_thickness_t padding;

    nk_direction_t direction;
    nk_sizing_t width;
    nk_sizing_t height;
    float gap;

    nk_align_t align_x;
    nk_align_t align_y;

    struct nk_view_t *parent;
    struct nk_view_t *first_child;
    struct nk_view_t *prev_sibling;
    struct nk_view_t *next_sibling;
} nk_view_t;

typedef enum
{
    NK_TEXT_NORMAL,
    NK_TEXT_BOLD
} nk_text_variant_t;

typedef struct
{
    nk_color_t color;
    float size;
    nk_text_variant_t variant;
} nk_text_info_t;

typedef struct
{
    nk_view_t view;

    const char *text;
    nk_text_info_t text_info;

    nk_void_callback_t press_callback;
} nk_button_t;

typedef struct
{
    nk_view_t view;

    const char *text;
    nk_text_info_t text_info;
} nk_label_t;

typedef enum
{
    NK_VIEW,
    NK_LABEL,
    NK_BUTTON
} nk_type_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

/* run nanokit */
int nk_run(nk_run_info_t *info, int argc, char **argv);

bool nk_window_create(nk_window_create_info_t *info, nk_window_t *window);

void nk_view_add_child(nk_view_t *parent, nk_view_t *child);
void nk_view_remove(nk_view_t *child);
void nk_view_insert_after(nk_view_t *sibling, nk_view_t *child);
void nk_view_insert_before(nk_view_t *sibling, nk_view_t *child);


#ifdef NANOKIT_MAIN

    void app_launched(void);

    #if (!DEBUG && _WIN32)
        int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
        {
            nk_run_info_t run_info = {
                .launch_callback = app_launched
            };

            return nk_run(&run_info, 0, NULL);
        }
    #else
        int main(int argc, char **argv)
        {
            nk_run_info_t run_info = {
                .launch_callback = app_launched
            };

            return nk_run(&run_info, argc, argv);
        }
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* NANOKIT_H */
