/***************************************************************
**
** Nanokit Source File
**
** File         :  cocoa.m
** Module       :  backend/cocoa
** Author       :  SH
** Created      :  2026-05-11 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Cocoa Backend
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>
#include <ui/view/view.h>

#include <resource/resource.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

#include <glad/glad.h>

#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

@interface app_delegate_t : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface gl_view_t : NSView
{
@public
    NSWindow *parent_window;
    int width;
    int height;
    int offset_y;
    int min_width;
    int min_height;
    int pointer_x;
    int pointer_y;
    bool pointer_down;
    view_context_t view_context;
    nk_view_t *root_view;
    bool dark_mode;
}
@end

@interface gl_layer_t : CAOpenGLLayer
{
@public
    gl_view_t *parent_view;
}
@end



/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static nk_run_info_t *run_info = NULL;
static app_delegate_t *app_delegate = NULL; 

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool backend_init(void)
{
     return true;
}

bool nk_window_create(nk_window_create_info_t *info, nk_window_t *window)
{
    NSRect frame = NSMakeRect(0, 0, info->start_width, info->start_height);
    
    NSWindow *ns_window = [[NSWindow alloc] initWithContentRect:frame
                                                styleMask:(NSWindowStyleMaskTitled |
                                                            NSWindowStyleMaskClosable |
                                                            NSWindowStyleMaskResizable |
                                                            NSWindowStyleMaskMiniaturizable |
                                                            NSWindowStyleMaskFullSizeContentView)
                                                backing:NSBackingStoreBuffered
                                                    defer:NO];
    ns_window.titlebarAppearsTransparent = YES;
    ns_window.titleVisibility = NSWindowTitleHidden;
    [ns_window center];
    [ns_window setTitle:[NSString stringWithUTF8String: info->title]];
    [ns_window makeKeyAndOrderFront:nil];
    [ns_window setDelegate:app_delegate];

    NSView *content = ns_window.contentView;
    content.wantsLayer = YES; /* layer-backed container helps with compositing */

    gl_view_t *gl_view = [[gl_view_t alloc] initWithFrame:content.bounds];
    gl_view.wantsLayer = YES;

    gl_layer_t *layer = [gl_layer_t new];
    layer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    layer.needsDisplayOnBoundsChange = YES;
    layer.contentsGravity = kCAGravityTopLeft;  // don't stretch old content
    layer->parent_view = gl_view;
    gl_view.layer = layer;
    gl_view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    gl_view->parent_window = ns_window;
    gl_view->width = info->start_width;
    gl_view->height = info->start_height;
    gl_view->min_width = info->min_width;
    gl_view->min_height = info->min_height;
    gl_view->root_view = info->root;
    
    [content addSubview:gl_view];

    printf("made window\n");

    *window = (nk_window_t)ns_window;
    
    return true;
}

int backend_run(nk_run_info_t *info, int argc, char **argv)
{
    run_info = info;
  
    @autoreleasepool
    {
        NSApplication *app = [NSApplication sharedApplication];
        app_delegate_t *delegate = [app_delegate_t new];
        app.delegate = delegate;
        [app run];
    }

    return 0;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/


@implementation app_delegate_t

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    app_delegate = self;
    printf("finished launching\n");
    run_info->launch_callback();
    printf("launch callback complete!\n");

}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

- (void)windowDidResize:(NSNotification *)notification
{   
    
}

@end


@implementation gl_view_t

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    NSRect backing = [self convertRectToBacking:self.bounds];
    width = (int)backing.size.width;
    height = (int)backing.size.height;

    self.layer.frame = self.bounds;
    [self.layer setNeedsDisplay];
}

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    // Remove old tracking areas if any exist
    for (NSTrackingArea *area in self.trackingAreas) {
        [self removeTrackingArea:area];
    }

    NSTrackingAreaOptions options = (NSTrackingActiveAlways | 
                                     NSTrackingMouseMoved | 
                                     NSTrackingMouseEnteredAndExited);
                                     
    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                        options:options
                                                          owner:self
                                                       userInfo:nil];
    [self addTrackingArea:area];
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    // app_cursor_moved(location.x, self.bounds.size.height - location.y);
    pointer_x = location.x;
    pointer_y = self.bounds.size.height - location.y;
    [self.layer setNeedsDisplay];
}

- (void)mouseEntered:(NSEvent *)event {
    /* don't care */
}

- (void)mouseExited:(NSEvent *)event {
    //app_cursor_lost();
    pointer_x = -1;
    pointer_y = -1;
    [self.layer setNeedsDisplay];
}

- (void)mouseDown:(NSEvent *)event {
    //app_button_changed(MOUSE_LEFT_BUTTON, true);
}

- (void)mouseUp:(NSEvent *)event {
    //app_button_changed(MOUSE_LEFT_BUTTON, false);
}

- (void)mouseDragged:(NSEvent *)event {
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    // app_cursor_moved(location.x, self.bounds.size.height - location.y);
}

- (void)rightMouseDown:(NSEvent *)event {
    //app_button_changed(MOUSE_RIGHT_BUTTON, true);
}

- (void)rightMouseUp:(NSEvent *)event {
    //app_button_changed(MOUSE_RIGHT_BUTTON, false);
}

- (void)rightMouseDragged:(NSEvent *)event {
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    //app_cursor_moved(location.x, self.bounds.size.height - location.y);
}

- (void)otherMouseDown:(NSEvent *)event {
    //app_button_changed(MOUSE_MIDDLE_BUTTON, true);
}

- (void)otherMouseUp:(NSEvent *)event {
    //app_button_changed(MOUSE_MIDDLE_BUTTON, false);
}

- (void)otherMouseDragged:(NSEvent *)event {
    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
    //app_cursor_moved(location.x, self.bounds.size.height - location.y);
}

- (void)scrollWheel:(NSEvent *)event {
    CGFloat deltaX = [event scrollingDeltaX];
    CGFloat deltaY = [event scrollingDeltaY];

    /* too sensitive */
    //app_axis_delta(0, deltaY / 10.0f);
}

- (void)resetCursorRects {
    [super resetCursorRects];
    
    // Fallback to arrow if nothing is set yet
    //NSCursor *cursor = current_app_cursor ? current_app_cursor : [NSCursor arrowCursor];
    
    //[self addCursorRect:self.bounds cursor:cursor];
}
// Ensure the view can receive keyboard/mouse focus
- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)viewDidChangeEffectiveAppearance
{
    NSAppearance *appearance = self.effectiveAppearance;
    NSAppearanceName name = [appearance bestMatchFromAppearancesWithNames:@[
        NSAppearanceNameAqua,
        NSAppearanceNameDarkAqua
    ]];
    dark_mode = [name isEqualToString:NSAppearanceNameDarkAqua];

    resource_set_system_appearance(dark_mode ? NK_RESOURCE_APPEARANCE_DARK : NK_RESOURCE_APPEARANCE_LIGHT);
}

@end

@implementation gl_layer_t

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pf
{
    CGLContextObj ctx = [super copyCGLContextForPixelFormat:pf];
    
    /* make current so glad/GL calls work */
    CGLSetCurrentContext(ctx);
    
    view_context_create_info_t view_create_info = {
        .default_font = "/System/Library/Fonts/SFNSRounded.ttf",
        .bold_font    = "/System/Library/Fonts/SFNSRounded.ttf",
    };

    if (!view_context_init(&self->parent_view->view_context, &view_create_info))
    {
        fprintf(stderr, "Failed to initialize renderer.\n");
    }

    
    return ctx;
}

- (BOOL)isAsynchronous { return YES; }

- (CGLPixelFormatObj)copyCGLPixelFormatForDisplayMask:(uint32_t)mask
{
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_GL4_Core,
        kCGLPFAColorSize,     (CGLPixelFormatAttribute)24,
        kCGLPFAAlphaSize,     (CGLPixelFormatAttribute)8,
        kCGLPFADepthSize,     (CGLPixelFormatAttribute)24,
        kCGLPFADoubleBuffer,
        kCGLPFAAccelerated,
        0
    };
    CGLPixelFormatObj pf;
    GLint npix;
    CGLChoosePixelFormat(attrs, &pf, &npix);
    return pf;
}

- (void)drawInCGLContext:(CGLContextObj)ctx
             pixelFormat:(CGLPixelFormatObj)pf
            forLayerTime:(CFTimeInterval)t
             displayTime:(const CVTimeStamp *)ts
{
    CGLSetCurrentContext(ctx);

    glViewport(0, 0, parent_view->width, parent_view->height);
    glDisable(GL_SCISSOR_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    view_context_render_info_t render_info = {
        .width = (float)parent_view->width,
        .height = (float)parent_view->height,
        .offset_y = (float)parent_view->offset_y,
        .dpr = 1.0f,
        .dark_mode = parent_view->dark_mode,
        .pointer_x = (float)parent_view->pointer_x,
        .pointer_y = (float)parent_view->pointer_y,
        .mouse_down = parent_view->pointer_down
    };

    view_context_render(&parent_view->view_context, parent_view->root_view, &render_info);
    
}

@end