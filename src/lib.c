/***************************************************************
**
** Nanokit Source File
**
** File         :  lib.c
** Module       :  nanokit
** Author       :  SH
** Created      :  2026-05-10 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Root Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

#include <backend/backend.h>
#include <resource/resource.h>
#include <ui/ui.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

int nk_run(nk_run_info_t *info, int argc, char **argv)
{
    if (!backend_init())
    {
        fprintf(stderr, "Failed to initialize backend.\n");
        return -1;
    }

    ui_set_info(info);

    resource_init();

    resource_load_default();

    int backend_code = backend_run(info, argc, argv);

    resource_destroy();

    return backend_code;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/
