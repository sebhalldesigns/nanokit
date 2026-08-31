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
#include <assert.h>
#include <stddef.h>

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

#define NK_DOCK_NODES_PER_SIDE_AREA (16U)
#define NK_DOCK_NODES_PER_MAIN_AREA (32U)

/* Number of values in nk_dock_tab_location_t. */
#define NK_DOCK_TAB_LOCATION_COUNT  (4U)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef enum
{
    NKRES_NONE,
    NKRES_COLOR_TEXT_PRIMARY,
    NKRES_COLOR_TEXT_SECONDARY,
    NKRES_COLOR_BACKGROUND_PRIMARY,
    NKRES_COLOR_BACKGROUND_SECONDARY,
    NKRES_COLOR_BACKGROUND_TERTIARY,
    NKRES_COLOR_BACKGROUND_POPUP,
    NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY,
    NKRES_COLOR_BACKGROUND_SPLITTER_ACTIVE,
    NKRES_COLOR_BACKGROUND_SPLITTER_INACTIVE,
    NKRES_COLOR_POPUP_BORDER,
    NKRES_COLOR_TAB_BORDER,
    NKRES_COLOR_BACKGROUND_SELECTED,
    NKRES_COLOR_RED,
    NKRES_SIZE_TEXT_PRIMARY,
    NKRES_SIZE_BUTTON_CORNER_RADIUS,
    NKRES_SIZE_POPUP_CORNER_RADIUS,
    NKRES_SIZE_SPLITTER_THICKNESS,
    NKRES_SIZE_SCROLLBAR_THICKNESS
} nk_resource_t;

/* Physical keys. Printable keys use their uppercase ASCII code, so NK_KEY_A is
   'A' and NK_KEY_COMMA is ','; named keys sit above the ASCII range. The key
   identifies the position on the keyboard, not the character it produces — so
   a binding works the same whatever the layout or shift state. */
typedef enum
{
    NK_KEY_UNKNOWN       = 0,

    NK_KEY_SPACE         = 32,
    NK_KEY_APOSTROPHE    = 39,
    NK_KEY_COMMA         = 44,
    NK_KEY_MINUS         = 45,
    NK_KEY_PERIOD        = 46,
    NK_KEY_SLASH         = 47,

    NK_KEY_0             = 48,
    NK_KEY_9             = 57,

    NK_KEY_SEMICOLON     = 59,
    NK_KEY_EQUAL         = 61,

    NK_KEY_A             = 65,
    NK_KEY_Z             = 90,

    NK_KEY_LEFT_BRACKET  = 91,
    NK_KEY_BACKSLASH     = 92,
    NK_KEY_RIGHT_BRACKET = 93,
    NK_KEY_GRAVE         = 96,

    NK_KEY_ESCAPE        = 256,
    NK_KEY_ENTER         = 257,
    NK_KEY_TAB           = 258,
    NK_KEY_BACKSPACE     = 259,
    NK_KEY_INSERT        = 260,
    NK_KEY_DELETE        = 261,
    NK_KEY_RIGHT         = 262,
    NK_KEY_LEFT          = 263,
    NK_KEY_DOWN          = 264,
    NK_KEY_UP            = 265,
    NK_KEY_PAGE_UP       = 266,
    NK_KEY_PAGE_DOWN     = 267,
    NK_KEY_HOME          = 268,
    NK_KEY_END           = 269,

    NK_KEY_F1            = 290,
    NK_KEY_F12           = 301
} nk_key_t;

typedef enum
{
    NK_MOD_NONE  = 0,
    NK_MOD_CTRL  = 1 << 0,
    NK_MOD_SHIFT = 1 << 1,
    NK_MOD_ALT   = 1 << 2,
    NK_MOD_SUPER = 1 << 3
} nk_modifier_t;

typedef struct
{
    uint32_t key;       /* an nk_key_t */
    uint32_t modifiers; /* bitmask of nk_modifier_t */
} nk_shortcut_t;

typedef enum
{
    NK_CURSOR_ARROW,
    NK_CURSOR_IBEAM,
    NK_CURSOR_HAND,
    NK_CURSOR_RESIZE_NS,
    NK_CURSOR_RESIZE_EW
} nk_cursor_t;


struct nk_view_t;
struct nk_menu_t;
struct nk_dock_group_t;
struct nk_dock_node_t;
struct nk_dock_t;
struct nk_button_t;
struct nk_tree_item_t;

typedef void (*nk_void_callback_t)(void);
typedef bool (*nk_bool_callback_t)(void);

/* note that paths are only valid for the duration of the callback */
typedef void (*nk_file_callback_t)(bool accepted, const char **paths, size_t path_count);
typedef void (*nk_directory_callback_t)(bool accepted, const char *directory_path);

typedef void (*nk_command_callback_t)(const char *command, const char *args);

typedef void (*nk_button_press_callback_t)(struct nk_button_t *button);

typedef void (*nk_tree_item_press_callback_t)(struct nk_tree_item_t *item);

typedef struct
{
    const char *application_id;
    nk_void_callback_t launch_callback;
    nk_command_callback_t command_callback;
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

typedef struct
{
    float x, y;
} nk_point_t;

typedef struct
{
    float x, y, width, height;
} nk_rect_t;

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
    uint32_t id;

    nk_resource_t background_resource;
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
    NK_TEXT_BOLD,
    NK_TEXT_ICON   /* glyphs from the embedded icon font (Bootstrap Icons) */
} nk_text_variant_t;

typedef struct
{
    nk_resource_t color_resource;
    float size;
    nk_text_variant_t variant;
} nk_text_info_t;

typedef enum
{
    NK_BUTTON_PRIMARY,
    NK_BUTTON_SECONDARY
} nk_button_type;

typedef struct nk_button_t
{
    nk_view_t view;

    const char *text;
    nk_text_info_t text_info;

    const char *secondary_text;
    nk_text_info_t secondary_text_info;

    const char *tooltip;

    nk_button_type button_type;

    void *user_data;

    nk_button_press_callback_t press_callback;
} nk_button_t;

typedef struct
{
    nk_view_t view;

    const char *text;
    nk_text_info_t text_info;

    /* Break only on newlines rather than at word boundaries. Text then
       overflows its container instead of reflowing, which is what a code or
       log line wants — an enclosing scroll view turns that into horizontal
       scrolling. */
    bool no_wrap;
} nk_label_t;

typedef struct
{
    nk_view_t view;

    /* When set, only the children intersecting the viewport are emitted, with
       spacers reserving the off-screen height. Requires uniform item_height. */
    bool virtualize;
    float item_height;

    /* Last-known-good scroll geometry. Clay_GetScrollContainerData is queried
       mid-layout and can momentarily report "not found" (its result hinges on a
       transient layout-arena slot that shifts when elements above the view
       change, e.g. on hover). Without a cache that collapses the virtual window
       to the fallback viewport and toggles the scrollbars — a visible flicker.
       Reusing these values on such frames keeps the view stable. */
    float geom_viewport_w, geom_viewport_h;
    float geom_content_w, geom_content_h;
    float geom_scroll_x, geom_scroll_y;
    bool  geom_valid;

    /* Width the content column was forced to hold last frame (see the FIT(.min)
       in scroll_view_render). Clay reports content dimensions one frame late, so
       while a pane is being narrowed the reported width is still the previous,
       wider minimum — overflow that does not exist. Remembering the forced
       minimum lets the horizontal bar ignore it. */
    float geom_forced_min_w;
} nk_scroll_view_t;

typedef struct nk_tree_item_t
{
    nk_view_t view;

    const char *text;
    nk_text_info_t text_info;

    int depth;
    bool is_folder;
    bool is_expanded;
    bool selected;

    void *user_data;

    nk_tree_item_press_callback_t press_callback;
} nk_tree_item_t;

typedef struct
{
    nk_view_t view;

    bool is_open;
    bool is_initial_press;
    struct nk_menu_t *open_menu;
} nk_menubar_t;

typedef struct
{
    nk_button_t button; /* used as a view for separator case */

    bool is_separator;

    const char *title;
    const char *shortcut;

    const char *command;
} nk_menu_entry_t;

typedef struct nk_menu_t
{
    nk_view_t view;
    nk_view_t popup;

    const char *heading;
    nk_menu_entry_t *entries;
    size_t entries_count;
    nk_menubar_t *parent_menubar;
} nk_menu_t;

typedef struct
{
    nk_menu_t *menus;
    size_t menus_count;
} nk_menubar_create_info_t;

typedef struct
{
    nk_view_t view;

    nk_direction_t direction;
    float *target;
    bool reversed;

    bool sizing;
    nk_point_t start_point;
    float start_target_size;

    bool proportional;
    float *reference;

    /* Lower bound (pixels) for the pane this splitter sizes. 0 = unbounded. For
       proportional splitters it is converted to a fraction of `reference`. */
    float min;
} nk_splitter_t;

typedef enum
{
    DOCK_TAB_MAIN_AREA,
    DOCK_TAB_LEFT_AREA,
    DOCK_TAB_RIGHT_AREA,
    DOCK_TAB_BOTTOM_AREA
} nk_dock_tab_location_t;

typedef struct
{
    nk_view_t view;

    const char* title;
    bool is_tool;

    struct nk_dock_group_t *dock_group;
} nk_dock_tab_t;

typedef struct
{
    nk_view_t view;

    nk_view_t tab_bar;
    nk_view_t content;
    /* Children of the content view are the tabs in the container.
     * Prevents arbitrary limits on number of tabs.
     */

    nk_dock_tab_t   *active_tab;
    nk_rect_t        content_bounds;
    struct nk_dock_node_t *node;
} nk_dock_group_t;

typedef enum
{
    NK_DOCK_NODE_TYPE_LEAF, /* a tab group */
    NK_DOCK_NODE_TYPE_SPLIT /* two child nodes + a splitter */
} nk_dock_node_type_t;

typedef struct nk_dock_node_t
{
    nk_view_t view;
    nk_dock_node_type_t node_type;

    union
    {
        struct
        {
            struct nk_dock_node_t *child_a;
            struct nk_dock_node_t *child_b;
            nk_splitter_t          splitter;
            float axis_reference;
            nk_direction_t direction;
        } split;

        nk_dock_group_t        group;
    } content;

    bool active;
    struct nk_dock_t *dock;
} nk_dock_node_t;

typedef struct
{
    nk_view_t view;

    nk_view_t left_area;
    nk_splitter_t left_splitter;
    nk_dock_node_t left_nodes[NK_DOCK_NODES_PER_SIDE_AREA];
    size_t left_nodes_count;

    nk_view_t central_area;

    nk_splitter_t right_splitter;
    nk_view_t right_area;
    nk_dock_node_t right_nodes[NK_DOCK_NODES_PER_SIDE_AREA];

    nk_splitter_t bottom_splitter;
    nk_view_t bottom_area;
    nk_dock_node_t bottom_nodes[NK_DOCK_NODES_PER_SIDE_AREA];

    nk_view_t main_area;
    nk_dock_node_t main_nodes[NK_DOCK_NODES_PER_MAIN_AREA];

    /* Whether each side area is shown. The main area is always present, so it
       has no flag. Hiding an area detaches it and its splitter from the view
       tree; the tabs it holds are untouched and return with it. */
    bool left_visible;
    bool right_visible;
    bool bottom_visible;

    /* Group most recently focused in each area, indexed by
       nk_dock_tab_location_t, and where nk_dock_add_tab() puts the next tab for
       that area. Revalidated against the area's node pool on use, so a group
       that has since been split away or collapsed is simply ignored. */
    struct nk_dock_group_t *recent_groups[NK_DOCK_TAB_LOCATION_COUNT];
} nk_dock_t;

typedef struct
{
    const char *app_title;
    nk_menubar_create_info_t *menubar_create_info;
} nk_workbench_create_info_t;

typedef struct
{
    nk_view_t view;

    nk_view_t titlebar;
    nk_dock_t dock;
    nk_view_t statusbar;

    nk_label_t brand_label;
    nk_menubar_t menubar;

    /* Eats the space between the menubar and the toggles, pinning them to the
       right edge of the titlebar. */
    nk_view_t titlebar_spacer;

    /* Left, bottom and right dock area toggles, in that visual order. */
    nk_button_t area_toggles[3];

} nk_workbench_t;

typedef enum
{
    NK_VIEW,
    NK_LABEL,
    NK_BUTTON,
    NK_SCROLL_VIEW,
    NK_TREE_ITEM,
    NK_MENU,
    NK_MENUBAR,
    NK_SPLITTER,
    NK_DOCK_NODE,
    NK_DOCK_GROUP,
    NK_DOCK,
    NK_WORKBENCH
} nk_type_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

/* run nanokit */
int nk_run(nk_run_info_t *info, int argc, char **argv);

void nk_io_open_files(bool single_file, nk_file_callback_t callback);
void nk_io_open_directory(nk_directory_callback_t callback);

/* Parse a chord such as "Ctrl-Shift-O", "Alt-F4" or "Ctrl-,". Modifier names
   (ctrl/control, shift, alt/option, cmd/command/super/meta/win) are
   case-insensitive and may be joined with '-' or '+'. Returns false, leaving
   `shortcut` untouched, if the text names no key this build knows. */
bool nk_shortcut_parse(const char *text, nk_shortcut_t *shortcut);

/* Bind a chord to a command, delivered through the run info's
   command_callback exactly as a menu item would deliver it. Binding a chord
   that is already bound replaces its command. `command` is stored by pointer
   and must outlive the binding — a string literal, as menu entries use.
   Returns false if the chord is unparseable or the table is full.

   Menu entries with both a shortcut and a command are registered
   automatically by nk_menubar_init(), so most callers never need this. */
bool nk_shortcut_register(const char *text, const char *command);

/* Remove a binding. Unbinding a chord that is not bound does nothing. */
void nk_shortcut_unregister(const char *text);

bool nk_window_create(nk_window_create_info_t *info, nk_window_t *window);
void nk_window_set_cursor(nk_window_t *window, nk_cursor_t cursor);
void nk_window_request_redraw(nk_window_t *window);

void nk_view_add_child(nk_view_t *parent, nk_view_t *child);
void nk_view_remove(nk_view_t *child);
void nk_view_insert_after(nk_view_t *sibling, nk_view_t *child);
void nk_view_insert_before(nk_view_t *sibling, nk_view_t *child);

void nk_workbench_init(nk_workbench_t *workbench, nk_workbench_create_info_t *create_info);

void nk_menubar_init(nk_menubar_t *menubar, nk_menubar_create_info_t *create_info);

void nk_menu_init(nk_menu_t *menu, nk_menubar_t *menubar);

void nk_button_init(nk_button_t *button, nk_button_press_callback_t callback);

void nk_scroll_view_init(nk_scroll_view_t *scroll_view);

void nk_tree_item_init(nk_tree_item_t *item, nk_tree_item_press_callback_t callback);

void nk_splitter_init(nk_splitter_t *splitter, float *target, nk_direction_t direction, bool reversed);
void nk_splitter_init_proportional(nk_splitter_t *splitter, float *target,
    nk_direction_t direction, bool reversed, float *reference);

void nk_dock_init(nk_dock_t *dock);

/* Dock `tab` into `location`, in whichever group of that area was focused most
   recently. Falls back to the area's first group when there is no such group —
   at startup, or after the remembered one has gone away. */
void nk_dock_add_tab(nk_dock_t *dock, nk_dock_tab_t *tab, nk_dock_tab_location_t location);

/* Make `tab` the visible tab of the group it belongs to. No-op for a tab that
   is not currently docked. */
void nk_dock_focus_tab(nk_dock_tab_t *tab);

/* Show or hide one of the dock's side areas. DOCK_TAB_MAIN_AREA is ignored —
   the main area cannot be hidden. Tabs in a hidden area keep their contents
   and reappear with it. */
void nk_dock_set_area_visible(nk_dock_t *dock, nk_dock_tab_location_t area, bool visible);

/* Flip an area's visibility. Returns the new state. */
bool nk_dock_toggle_area(nk_dock_t *dock, nk_dock_tab_location_t area);

bool nk_dock_is_area_visible(const nk_dock_t *dock, nk_dock_tab_location_t area);

/* Undock `tab`, activating a neighbour in its place. Mirrors what the tab's
   close button does, minus collapsing a group that becomes empty. */
void nk_dock_close_tab(nk_dock_tab_t *tab);

/* True while `tab` is docked — a tab the user has closed reports false. */
bool nk_dock_tab_is_open(const nk_dock_tab_t *tab);

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
