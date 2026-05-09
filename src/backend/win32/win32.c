/***************************************************************
**
** Nanokit Source File
**
** File         :  win32.c
** Module       :  backend/win32
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Win32 Backend
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>
#include <ui/context/context.h>


#include <string.h>
#include <wchar.h>

#include <dwmapi.h>

#include <glad/glad.h>
#include "wglext.h"

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/* See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt for all values */
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

/* See https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt for all values */
#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013
#define WGL_COLOR_BITS_ARB                        0x2014
#define WGL_DEPTH_BITS_ARB                        0x2022
#define WGL_STENCIL_BITS_ARB                      0x2023

#define WGL_FULL_ACCELERATION_ARB                 0x2027
#define WGL_TYPE_RGBA_ARB                         0x202B

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef HGLRC WINAPI wglCreateContextAttribsARB_type(HDC hdc, HGLRC hShareContext,
        const int *attribList);
typedef BOOL WINAPI wglChoosePixelFormatARB_type(HDC hdc, const int *piAttribIList,
        const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

typedef struct {
    HWND hwnd;
    HDC gldc;
    int width;
    int height;
    HCURSOR cursor;
    int pointer_x;
    int pointer_y;
    bool pointer_down;
    ui_context_t render_context;
    nk_view_t *root_view;
} win32_window_data_t;


/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static HINSTANCE instance_handle;
static WNDCLASSW window_class;

static int pixel_format;
static PIXELFORMATDESCRIPTOR pfd = { 0 };

static wglCreateContextAttribsARB_type *wglCreateContextAttribsARB = NULL;
static wglChoosePixelFormatARB_type *wglChoosePixelFormatARB = NULL;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
static PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static bool backend_init();

static LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);

static win32_window_data_t* get_window_data(HWND hwnd);

static void apply_dwm_frame(HWND hwnd);
static void set_process_dpi_awareness(void);
static void print_last_error(const char *context);

static const char* wide_to_utf8(const wchar_t *wstr);
static const wchar_t* utf8_to_wide(const char *str);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool nk_window_create(nk_window_create_info_t *info, nk_window_t *window)
{
    const wchar_t *wtitle = utf8_to_wide(info->title);

    HWND win32_window = CreateWindowExW(
        0,
        window_class.lpszClassName,
        wtitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        info->start_width,
        info->start_height,
        0,
        0,
        instance_handle,
        0
    );

    free((void*)wtitle);

    if (!win32_window)
    {
        fprintf(stderr, "Failed to create window.\n");
        print_last_error("CreateWindowExW(NANOKIT_WINDOW_CLASS)");
        return false;
    }

    win32_window_data_t* data = (win32_window_data_t*)calloc(1, sizeof(win32_window_data_t));
    data->hwnd = win32_window;
    data->gldc = GetDC(win32_window);
    RECT client_rect = {0};
    GetClientRect(win32_window, &client_rect);
    data->width = client_rect.right - client_rect.left;
    data->height = client_rect.bottom - client_rect.top;
    data->cursor = LoadCursorW(NULL, IDC_ARROW);
    data->root_view = info->root;

    const static int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };

    UINT num_formats;
    wglChoosePixelFormatARB(data->gldc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    if (!num_formats) {
        fprintf(stderr, "Failed to set the OpenGL 3.3 pixel format.\n");
    }

    DescribePixelFormat(data->gldc, pixel_format, sizeof(pfd), &pfd);
    if (!SetPixelFormat(data->gldc, pixel_format, &pfd))
    {
        fprintf(stderr, "Failed to set the OpenGL 3.3 pixel format.\n");
    }

    const static int gl33_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    HGLRC gl33_context = wglCreateContextAttribsARB(data->gldc, 0, gl33_attribs);
    if (!gl33_context)
    {
        fprintf(stderr, "Failed to create OpenGL 3.3 context.\n");
    }

    if (!wglMakeCurrent(data->gldc, gl33_context))
    {
        fprintf(stderr, "Failed to activate OpenGL 3.3 rendering context.\n");
    }

    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(1);   /* enable vsync */
    }

    SetWindowLongPtr(win32_window, GWLP_USERDATA, (LONG_PTR)data);

    if (!ui_context_init(&data->render_context, "C:/Windows/Fonts/segoeui.ttf"))
    {
        fprintf(stderr, "Failed to initialize renderer.\n");
    }

    ShowWindow(data->hwnd, SW_SHOW);
    UpdateWindow(data->hwnd);

    *window = (nk_window_t)win32_window;

    return true;
}

int nk_run(nk_run_info_t *info, int argc, char **argv)
{
    printf("Hello world!\n");

    if (!backend_init())
    {
        fprintf(stderr, "Failed to initialize backend.\n");
        return -1;
    }

    info->launch_callback();

    bool running = true;

    MSG msg;

    while (running)
    {
        MSG msg;

        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return 0;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/


static bool backend_init()
{
    instance_handle = GetModuleHandle(NULL);
    set_process_dpi_awareness();

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    memset(&window_class, 0, sizeof(window_class));
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = window_procedure;
    window_class.hInstance = instance_handle;
    window_class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    window_class.hbrBackground = NULL;
    window_class.lpszClassName = L"NANOKIT_WINDOW_CLASS";

    if (!RegisterClassW(&window_class))
    {
        fprintf(stderr, "Failed to register NANOKIT_WINDOW_CLASS.");
        return false;
    }

    WNDCLASSW gl_window_class = { 0 };
    gl_window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    gl_window_class.lpfnWndProc = DefWindowProcW;
    gl_window_class.hInstance = GetModuleHandle(0);
    gl_window_class.lpszClassName = L"NANOKIT_GL_INIT_WINDOW_CLASS";

    if (!RegisterClassW(&gl_window_class))
    {
        fprintf(stderr, "Failed to register OpenGL init window.");
        return false;
    }

    HWND dummy_window = CreateWindowExW(
        0,
        gl_window_class.lpszClassName,
        L"OpenGL Init Window",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        gl_window_class.hInstance,
        0
    );

    if (!dummy_window)
    {
        fprintf(stderr, "Failed to create OpenGL init window.");
        return false;
    }

    HDC dummy_dc = GetDC(dummy_window);

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;


    pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    if (!pixel_format)
    {
        fprintf(stderr, "Failed to find a suitable pixel format.");
        return false;
    }

    if (!SetPixelFormat(dummy_dc, pixel_format, &pfd))
    {
        fprintf(stderr, "Failed to set the pixel format.");
        return false;
    }

    HGLRC dummy_context = wglCreateContext(dummy_dc);
    if (!dummy_context)
    {
        fprintf(stderr, "Failed to create an OpenGL rendering context.");
        return false;
    }

    if (!wglMakeCurrent(dummy_dc, dummy_context))
    {
        fprintf(stderr, "Failed to activate OpenGL rendering context.");
        return false;
    }

    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_type*)wglGetProcAddress(
        "wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_type*)wglGetProcAddress(
        "wglChoosePixelFormatARB");

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
        wglGetProcAddress("wglSwapIntervalEXT");

    wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)
        wglGetProcAddress("wglGetSwapIntervalEXT");

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);

    return true;
}

static LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    win32_window_data_t* data = get_window_data(window);

    /* Let DWM handle caption button hit testing first */
    LRESULT dwm_result = 0;
    if (DwmDefWindowProc(window, msg, wparam, lparam, &dwm_result))
    {
        return dwm_result;
    }


    switch (msg)
    {
        #if 0
        case WM_CREATE:
            apply_dwm_frame(window);
            return 0;

        case WM_DWMCOMPOSITIONCHANGED:
            apply_dwm_frame(window);
            return 0;
        #endif

        case WM_CLOSE:
        {
            DestroyWindow(window);
            return 0;
        }

        #if 0
        case WM_NCCALCSIZE:
        {
            if (wparam == TRUE)
            {
                NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lparam;

                UINT dpi = GetDpiForWindow(window);
                int frame_x = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
                int frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
                int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

                // Keep resize borders working.
                params->rgrc[0].left   += frame_x + padding;
                params->rgrc[0].right  -= frame_x + padding;
                params->rgrc[0].bottom -= frame_y + padding;

                /*
                ** N.B - changing rgrc[0].bottom seems to break the caption buttons
                ** from when the window is maximised. Don't touch it and handle
                ** in the client area rendering code instead.
                */

                return 0;
            }

            return DefWindowProcW(window, msg, wparam, lparam);
        }

        case WM_NCHITTEST:
        {

            break;
        }
        #endif

        case WM_ERASEBKGND:
        {
            return 1;
        } break;

        case WM_SIZE:
        {
            uint32_t width = LOWORD(lparam);
            uint32_t height = HIWORD(lparam);

            if (wparam != SIZE_MINIMIZED)
            {
                data->width = width;
                data->height = height;

                InvalidateRect(window, NULL, FALSE);
            }

            return 0;
        }

        case WM_PAINT:
        {
            if (data)
            {

                PAINTSTRUCT ps;
                BeginPaint(window, &ps);

                glViewport(0, 0, data->width, data->height);
                glDisable(GL_SCISSOR_TEST);
                //glClearColor(38.0f/255.0f, 38.0f/255.0f, 46.0f/255.0f, 0.0f);
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                ui_context_render(&data->render_context, data->root_view, (float)data->width, (float)data->height, 1.0f,
                        (float)data->pointer_x, (float)data->pointer_y, data->pointer_down);

                SwapBuffers(data->gldc);

                EndPaint(window, &ps);

            }

            return 0;
        }

        case WM_NCDESTROY:
        {
            if (data)
            {
                SetWindowLongPtr(window, GWLP_USERDATA, 0);
            }

            PostQuitMessage(0);
            return 0;
        }

        case WM_MOUSEMOVE:
            if (data)
            {
                data->pointer_x = GET_X_LPARAM(lparam);
                data->pointer_y = GET_Y_LPARAM(lparam);
            }
            InvalidateRect(window, NULL, FALSE);
            return 0;

        case WM_SETCURSOR:
        {
            if (LOWORD(lparam) == HTCLIENT)
            {
                SetCursor(data->cursor);
                return TRUE;
            }
        } break;

        case WM_LBUTTONDOWN:
            if (data)
            {
                data->pointer_down = true;
            }
            SetCapture(window);
            InvalidateRect(window, NULL, FALSE);
            return 0;

        case WM_LBUTTONUP:
            if (data)
            {
                data->pointer_down = false;
            }
            ReleaseCapture();
            InvalidateRect(window, NULL, FALSE);
            return 0;

        case WM_RBUTTONDOWN:
            InvalidateRect(window, NULL, FALSE);
            return 0;

        case WM_RBUTTONUP:
            InvalidateRect(window, NULL, FALSE);
            return 0;

        case WM_MOUSEWHEEL:
            InvalidateRect(window, NULL, FALSE);
            return 0;

        case WM_CHAR:
            InvalidateRect(window, NULL, FALSE);
            return 0;

        case WM_KEYDOWN:
        case WM_KEYUP:
        {

            InvalidateRect(window, NULL, FALSE);
            return 0;
        }

        default:
        {
            /* do nothing */
        } break;
    }

    return DefWindowProcW(window, msg, wparam, lparam);
}

static win32_window_data_t* get_window_data(HWND hwnd)
{
    return (win32_window_data_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

static void apply_dwm_frame(HWND hwnd)
{
    MARGINS margins = { 0, 0, 32, 0 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
}

static void set_process_dpi_awareness(void)
{
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE ((HANDLE)-3)
#endif

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32)
        return;

    typedef BOOL (WINAPI *SetProcessDpiAwarenessContextProc)(HANDLE);
    SetProcessDpiAwarenessContextProc set_dpi_context =
        (SetProcessDpiAwarenessContextProc)GetProcAddress(user32, "SetProcessDpiAwarenessContext");

    if (set_dpi_context)
    {
        if (set_dpi_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            return;
        if (set_dpi_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
            return;
    }

    typedef BOOL (WINAPI *SetProcessDPIAwareProc)(void);
    SetProcessDPIAwareProc set_dpi_aware =
        (SetProcessDPIAwareProc)GetProcAddress(user32, "SetProcessDPIAware");
    if (set_dpi_aware)
        set_dpi_aware();
}

static void print_last_error(const char *context)
{
    DWORD error = GetLastError();
    LPSTR message = NULL;
    DWORD length = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message,
        0,
        NULL
    );

    if (length > 0 && message)
    {
        fprintf(stderr, "%s failed with error %lu: %s\n", context, (unsigned long)error, message);
        LocalFree(message);
    }
    else
    {
        fprintf(stderr, "%s failed with error %lu.\n", context, (unsigned long)error);
    }
}


static const char* wide_to_utf8(const wchar_t *wstr)
{
    int size = WideCharToMultiByte(
            CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

    if (size <= 0)
    {
        return NULL;
    }

    char *utf8_str = (char*)malloc(size);

    if (!utf8_str)
    {
        return NULL;
    }

    if (!WideCharToMultiByte(
                CP_UTF8, 0, wstr, -1,
                utf8_str, size, NULL, NULL))
    {
        free(utf8_str);
        return NULL;
    }

    return utf8_str;
}

static const wchar_t* utf8_to_wide(const char *str)
{
    int size = MultiByteToWideChar(
            CP_UTF8, 0, str, -1, NULL, 0);

    if (size <= 0)
    {
        return NULL;
    }

    wchar_t *wstr = (wchar_t*)malloc(size * sizeof(wchar_t));

    if (!wstr)
    {
        return NULL;
    }

    if (!MultiByteToWideChar(
                CP_UTF8, 0, str, -1,
                wstr, size))
    {
        free(wstr);
        return NULL;
    }

    return wstr;
}
