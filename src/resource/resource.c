/***************************************************************
**
** Nanokit Source File
**
** File         :  resource.c
** Module       :  ui/resource
** Author       :  SH
** Created      :  2026-05-10 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Resource Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>
#include <resource/resource.h>

#include <klib/khash.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/* Define hashmap types: uint32_t key -> nk_color_t / float value */
KHASH_MAP_INIT_INT(dynamic_color_hash, nk_dynamic_color_t)
KHASH_MAP_INIT_INT(float_hash, float)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static nk_resource_appearance_mode_t appearance;
static khash_t(dynamic_color_hash) *dynamic_color_resources = NULL;
static khash_t(float_hash) *float_resources = NULL;


/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

void resource_init(void)
{
    /* Default to light mode */
    appearance = NK_RESOURCE_APPEARANCE_LIGHT;
    dynamic_color_resources = kh_init(dynamic_color_hash);
    float_resources = kh_init(float_hash);
}

void resource_destroy(void)
{
    kh_destroy(dynamic_color_hash, dynamic_color_resources);
    kh_destroy(float_hash, float_resources);
    dynamic_color_resources = NULL;
    float_resources = NULL;
}

void resource_set_system_appearance(nk_resource_appearance_mode_t new_appearance)
{
    appearance = new_appearance;
}

void resource_set_dynamic_color(uint32_t key, nk_dynamic_color_t value)
{
    int ret;
    khiter_t k = kh_put(dynamic_color_hash, dynamic_color_resources, key, &ret);
    kh_value(dynamic_color_resources, k) = value;
}

void resource_set_float(uint32_t key, float value)
{
    int ret;
    khiter_t k = kh_put(float_hash, float_resources, key, &ret);
    kh_value(float_resources, k) = value;
}

nk_color_t resource_get_dynamic_color(uint32_t key)
{
    if (key == NKRES_NONE)
    {
        return (nk_color_t){ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    khiter_t k = kh_get(dynamic_color_hash, dynamic_color_resources, key);

    if (k != kh_end(dynamic_color_resources))
    {
        return kh_value(dynamic_color_resources, k).colors[appearance];
    }

    /* Fallback — obvious magenta so missing keys are visible */
    return (nk_color_t){ 1.0f, 0.0f, 1.0f, 1.0f };
}

float resource_get_float(uint32_t key)
{
    if (key == NKRES_NONE)
    {
        return 0.0f;
    }

    khiter_t k = kh_get(float_hash, float_resources, key);

    if (k != kh_end(float_resources))
    {
        return kh_value(float_resources, k);
    }

    return 0.0f;
}

void resource_load_default(void)
{


    resource_set_dynamic_color(NKRES_COLOR_TEXT_PRIMARY, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.0f, 0.0f, 0.0f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 1.0f, 1.0f, 1.0f, 1.0f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_TEXT_SECONDARY, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.0f, 0.0f, 0.0f, 0.5f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 1.0f, 1.0f, 1.0f, 0.5f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_BACKGROUND_PRIMARY, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 1.0f, 1.0f, 1.0f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.0f, 0.0f, 0.0f, 1.0f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_BACKGROUND_SECONDARY, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.95f, 0.95f, 0.95f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.1f, 0.1f, 0.12f, 1.0f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_BACKGROUND_TERTIARY, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.9f, 0.9f, 0.9f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.15f, 0.15f, 0.18f, 1.0f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_BACKGROUND_POPUP, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.9f, 0.9f, 0.9f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.21f, 0.21f, 0.22f, 1.0f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_BACKGROUND_BUTTON_SECONDARY, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.9f, 0.9f, 0.9f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.3f, 0.3f, 0.3f, 0.4f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_BACKGROUND_SPLITTER_ACTIVE, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.9f, 0.9f, 0.9f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.15f, 0.15f, 0.18f, 1.0f }
        }
    });

    resource_set_dynamic_color(NKRES_COLOR_BACKGROUND_SPLITTER_INACTIVE, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 0.9f, 0.9f, 0.9f, 0.5f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.15f, 0.15f, 0.18f, 0.5f }
        }
    });


    resource_set_dynamic_color(NKRES_COLOR_RED, (nk_dynamic_color_t){
        .colors = {
            [NK_RESOURCE_APPEARANCE_LIGHT] = (nk_color_t){ 1.0f, 0.1f, 0.1f, 1.0f },
            [NK_RESOURCE_APPEARANCE_DARK] = (nk_color_t){ 0.9f, 0.1f, 0.1f, 1.0f }
        }
    });

    resource_set_float(NKRES_SIZE_TEXT_PRIMARY, 12.0f);
    resource_set_float(NKRES_SIZE_BUTTON_CORNER_RADIUS, 3.0f);
    resource_set_float(NKRES_SIZE_POPUP_CORNER_RADIUS, 4.0f);
    resource_set_float(NKRES_SIZE_SPLITTER_THICKNESS, 2.0f);
}


/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
