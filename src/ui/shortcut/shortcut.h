/***************************************************************
**
** Nanokit Header File
**
** File         :  shortcut.h
** Module       :  ui/shortcut
** Author       :  SH
** Created      :  2026-08-31 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Keyboard Shortcut Interface Definition
**
***************************************************************/

#ifndef SHORTCUT_H
#define SHORTCUT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <nanokit.h>

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

/* Offer a key press to the registered shortcuts. Returns true if one matched
   and its command was dispatched, so the backend can treat the key as
   consumed. Called by backends from their key handler. */
bool shortcut_handle_key(nk_key_t key, uint32_t modifiers);

#ifdef __cplusplus
}
#endif

#endif /* SHORTCUT_H */
