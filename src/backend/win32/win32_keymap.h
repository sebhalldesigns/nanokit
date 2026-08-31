/***************************************************************
**
** Nanokit Header File
**
** File         :  win32_keymap.h
** Module       :  backend/win32
** Author       :  SH
** Created      :  2026-08-31 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Win32 Virtual-Key Translation
**
***************************************************************/

#ifndef WIN32_KEYMAP_H
#define WIN32_KEYMAP_H

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

/* Translate a Windows virtual-key code into nanokit's key code.
   Letters, digits and space share their values with VK codes; everything else
   is spelled out. Returns NK_KEY_UNKNOWN for keys with no nanokit equivalent.

   Kept in a header of its own so it can be exercised from a host build that
   has no Windows toolchain — see the keymap test, which supplies the VK_*
   values and includes this file directly. */
static inline nk_key_t win32_key_from_vk(unsigned int vk)
{
    /* VK codes for letters and digits are their ASCII values, which is exactly
       what nanokit uses for them. */
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9'))
    {
        return (nk_key_t)vk;
    }

    if (vk >= VK_F1 && vk <= VK_F12)
    {
        return (nk_key_t)(NK_KEY_F1 + (vk - VK_F1));
    }

    switch (vk)
    {
        case VK_SPACE:      return NK_KEY_SPACE;

        case VK_OEM_7:      return NK_KEY_APOSTROPHE;
        case VK_OEM_COMMA:  return NK_KEY_COMMA;
        case VK_OEM_MINUS:  return NK_KEY_MINUS;
        case VK_OEM_PERIOD: return NK_KEY_PERIOD;
        case VK_OEM_2:      return NK_KEY_SLASH;
        case VK_OEM_1:      return NK_KEY_SEMICOLON;
        case VK_OEM_PLUS:   return NK_KEY_EQUAL;
        case VK_OEM_4:      return NK_KEY_LEFT_BRACKET;
        case VK_OEM_5:      return NK_KEY_BACKSLASH;
        case VK_OEM_6:      return NK_KEY_RIGHT_BRACKET;
        case VK_OEM_3:      return NK_KEY_GRAVE;

        case VK_ESCAPE:     return NK_KEY_ESCAPE;
        case VK_RETURN:     return NK_KEY_ENTER;
        case VK_TAB:        return NK_KEY_TAB;
        case VK_BACK:       return NK_KEY_BACKSPACE;
        case VK_INSERT:     return NK_KEY_INSERT;
        case VK_DELETE:     return NK_KEY_DELETE;
        case VK_RIGHT:      return NK_KEY_RIGHT;
        case VK_LEFT:       return NK_KEY_LEFT;
        case VK_DOWN:       return NK_KEY_DOWN;
        case VK_UP:         return NK_KEY_UP;
        case VK_PRIOR:      return NK_KEY_PAGE_UP;
        case VK_NEXT:       return NK_KEY_PAGE_DOWN;
        case VK_HOME:       return NK_KEY_HOME;
        case VK_END:        return NK_KEY_END;

        default:            return NK_KEY_UNKNOWN;
    }
}

/* Current modifier state as an nk_modifier_t bitmask. */
static inline uint32_t win32_modifiers(void)
{
    uint32_t modifiers = NK_MOD_NONE;

    if (GetKeyState(VK_CONTROL) & 0x8000) { modifiers |= NK_MOD_CTRL;  }
    if (GetKeyState(VK_SHIFT)   & 0x8000) { modifiers |= NK_MOD_SHIFT; }
    if (GetKeyState(VK_MENU)    & 0x8000) { modifiers |= NK_MOD_ALT;   }

    if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000))
    {
        modifiers |= NK_MOD_SUPER;
    }

    return modifiers;
}

#ifdef __cplusplus
}
#endif

#endif /* WIN32_KEYMAP_H */
