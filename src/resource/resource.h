/***************************************************************
**
** Nanokit Header File
**
** File         :  resource.h
** Module       :  ui/resource
** Author       :  SH
** Created      :  2026-05-10 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Resource Interface Definition
**
***************************************************************/

#ifndef RESOURCE_H
#define RESOURCE_H

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

typedef enum
{
    NK_RESOURCE_APPEARANCE_LIGHT,
    NK_RESOURCE_APPEARANCE_DARK,
    NK_RESOURCE_APPERANCE_MAX
} nk_resource_appearance_mode_t;

typedef struct
{
    nk_color_t colors[NK_RESOURCE_APPERANCE_MAX];
} nk_dynamic_color_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

void resource_init(void);
void resource_destroy(void);

void resource_set_system_appearance(nk_resource_appearance_mode_t appearance);

void resource_set_dynamic_color(uint32_t key, nk_dynamic_color_t value);
void resource_set_float(uint32_t key, float value);

nk_color_t resource_get_dynamic_color(uint32_t key);
float resource_get_float(uint32_t key);

void resource_load_default(void);

#ifdef __cplusplus
}
#endif

#endif /* RESOURCE_H */
