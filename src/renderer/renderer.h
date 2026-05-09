/***************************************************************
**
** Nanokit Header File
**
** File         :  renderer.h
** Module       :  renderer
** Author       :  SH
** Created      :  2026-05-09 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Renderer Interface Definition
**
***************************************************************/

#ifndef RENDERER_H
#define RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef uintptr_t render_context_t;

bool renderer_init(render_context_t *context);

void renderer_render(render_context_t *context, float width, float height, float dpr);

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* RENDERER_H */
