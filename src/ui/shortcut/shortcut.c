/***************************************************************
**
** Nanokit Source File
**
** File         :  shortcut.c
** Module       :  ui/shortcut
** Author       :  SH
** Created      :  2026-08-31 (YYYY-MM-DD)
** License      :  MIT
** Description  :  Nanokit Keyboard Shortcut Implementation
**
***************************************************************/

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include "ui/shortcut/shortcut.h"

#include <nanokit.h>

#include <ui/ui.h>

#include <ctype.h>
#include <string.h>

/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#define SHORTCUT_MAX_BINDINGS (64U)

/* Modifiers a chord may carry. Anything else a backend reports (caps lock,
   num lock) is masked off before matching, so a lit lock key cannot stop a
   shortcut from firing. */
#define SHORTCUT_MOD_MASK \
    (NK_MOD_CTRL | NK_MOD_SHIFT | NK_MOD_ALT | NK_MOD_SUPER)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef struct
{
    const char *name;
    uint32_t    value;
} shortcut_name_t;

typedef struct
{
    nk_shortcut_t shortcut;
    const char   *command;
    bool          in_use;
} shortcut_binding_t;

/***************************************************************
** MARK: STATIC VARIABLES
***************************************************************/

static shortcut_binding_t bindings[SHORTCUT_MAX_BINDINGS];

static const shortcut_name_t modifier_names[] = {
    { "ctrl",    NK_MOD_CTRL  },
    { "control", NK_MOD_CTRL  },
    { "shift",   NK_MOD_SHIFT },
    { "alt",     NK_MOD_ALT   },
    { "option",  NK_MOD_ALT   },
    { "cmd",     NK_MOD_SUPER },
    { "command", NK_MOD_SUPER },
    { "super",   NK_MOD_SUPER },
    { "meta",    NK_MOD_SUPER },
    { "win",     NK_MOD_SUPER }
};

static const shortcut_name_t key_names[] = {
    { "space",     NK_KEY_SPACE     },
    { "escape",    NK_KEY_ESCAPE    },
    { "esc",       NK_KEY_ESCAPE    },
    { "enter",     NK_KEY_ENTER     },
    { "return",    NK_KEY_ENTER     },
    { "tab",       NK_KEY_TAB       },
    { "backspace", NK_KEY_BACKSPACE },
    { "insert",    NK_KEY_INSERT    },
    { "delete",    NK_KEY_DELETE    },
    { "del",       NK_KEY_DELETE    },
    { "right",     NK_KEY_RIGHT     },
    { "left",      NK_KEY_LEFT      },
    { "down",      NK_KEY_DOWN      },
    { "up",        NK_KEY_UP        },
    { "pageup",    NK_KEY_PAGE_UP   },
    { "pagedown",  NK_KEY_PAGE_DOWN },
    { "home",      NK_KEY_HOME      },
    { "end",       NK_KEY_END       },
    { "comma",     NK_KEY_COMMA     },
    { "period",    NK_KEY_PERIOD    },
    { "dot",       NK_KEY_PERIOD    },
    { "minus",     NK_KEY_MINUS     },
    { "plus",      NK_KEY_EQUAL     },
    { "equal",     NK_KEY_EQUAL     },
    { "slash",     NK_KEY_SLASH     },
    { "backslash", NK_KEY_BACKSLASH },
    { "grave",     NK_KEY_GRAVE     },
    { "f1",  NK_KEY_F1      },
    { "f2",  NK_KEY_F1 + 1  },
    { "f3",  NK_KEY_F1 + 2  },
    { "f4",  NK_KEY_F1 + 3  },
    { "f5",  NK_KEY_F1 + 4  },
    { "f6",  NK_KEY_F1 + 5  },
    { "f7",  NK_KEY_F1 + 6  },
    { "f8",  NK_KEY_F1 + 7  },
    { "f9",  NK_KEY_F1 + 8  },
    { "f10", NK_KEY_F1 + 9  },
    { "f11", NK_KEY_F1 + 10 },
    { "f12", NK_KEY_F12     }
};

/***************************************************************
** MARK: STATIC FUNCTION DEFS
***************************************************************/

static bool name_matches(const char *name, const char *text, size_t length);
static bool take_modifier(const char **text, uint32_t *modifiers);
static uint32_t parse_key(const char *text);
static shortcut_binding_t *find_binding(nk_shortcut_t shortcut);

/***************************************************************
** MARK: PUBLIC FUNCTIONS
***************************************************************/

bool nk_shortcut_parse(const char *text, nk_shortcut_t *shortcut)
{
    if (!text || !shortcut)
    {
        return false;
    }

    uint32_t modifiers = NK_MOD_NONE;

    /* Consume modifier prefixes; whatever is left names the key. Because a
       prefix is only taken when a separator follows it, "Ctrl--" leaves "-"
       behind and binds the minus key rather than parsing to nothing. */
    while (take_modifier(&text, &modifiers))
    {
        /* keep going */
    }

    uint32_t key = parse_key(text);

    if (key == NK_KEY_UNKNOWN)
    {
        return false;
    }

    shortcut->key = key;
    shortcut->modifiers = modifiers;

    return true;
}

bool nk_shortcut_register(const char *text, const char *command)
{
    nk_shortcut_t shortcut;

    if (!command || !nk_shortcut_parse(text, &shortcut))
    {
        return false;
    }

    shortcut_binding_t *binding = find_binding(shortcut);

    if (!binding)
    {
        for (size_t i = 0; i < SHORTCUT_MAX_BINDINGS; i++)
        {
            if (!bindings[i].in_use)
            {
                binding = &bindings[i];
                break;
            }
        }
    }

    if (!binding)
    {
        return false;
    }

    binding->shortcut = shortcut;
    binding->command = command;
    binding->in_use = true;

    return true;
}

void nk_shortcut_unregister(const char *text)
{
    nk_shortcut_t shortcut;

    if (!nk_shortcut_parse(text, &shortcut))
    {
        return;
    }

    shortcut_binding_t *binding = find_binding(shortcut);

    if (binding)
    {
        binding->in_use = false;
        binding->command = NULL;
    }
}

bool shortcut_handle_key(nk_key_t key, uint32_t modifiers)
{
    nk_shortcut_t pressed = {
        .key = (uint32_t)key,
        .modifiers = modifiers & SHORTCUT_MOD_MASK
    };

    shortcut_binding_t *binding = find_binding(pressed);

    if (!binding)
    {
        return false;
    }

    nk_run_info_t *info = ui_get_info();

    if (!info || !info->command_callback)
    {
        return false;
    }

    info->command_callback(binding->command, NULL);

    return true;
}

/***************************************************************
** MARK: STATIC FUNCTIONS
***************************************************************/

/* Case-insensitive compare of a table name against `length` bytes of text. */
static bool name_matches(const char *name, const char *text, size_t length)
{
    if (strlen(name) != length)
    {
        return false;
    }

    for (size_t i = 0; i < length; i++)
    {
        if (tolower((unsigned char)name[i]) != tolower((unsigned char)text[i]))
        {
            return false;
        }
    }

    return true;
}

/* Take one leading modifier name and its separator, if present. */
static bool take_modifier(const char **text, uint32_t *modifiers)
{
    const char *cursor = *text;

    /* The separator must be found beyond the first character so that a chord
       whose key is itself '-' or '+' is not mistaken for a separator. */
    const char *separator = NULL;

    for (const char *c = cursor + 1; *c; c++)
    {
        if (*c == '-' || *c == '+')
        {
            separator = c;
            break;
        }
    }

    if (!separator)
    {
        return false;
    }

    size_t length = (size_t)(separator - cursor);

    for (size_t i = 0; i < sizeof(modifier_names) / sizeof(modifier_names[0]); i++)
    {
        if (name_matches(modifier_names[i].name, cursor, length))
        {
            *modifiers |= modifier_names[i].value;
            *text = separator + 1;
            return true;
        }
    }

    return false;
}

static uint32_t parse_key(const char *text)
{
    size_t length = strlen(text);

    if (length == 0)
    {
        return NK_KEY_UNKNOWN;
    }

    if (length == 1)
    {
        unsigned char c = (unsigned char)toupper((unsigned char)text[0]);

        /* Letters and digits map onto their ASCII code directly. */
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
        {
            return c;
        }

        switch (c)
        {
            case ' ':  return NK_KEY_SPACE;
            case '\'': return NK_KEY_APOSTROPHE;
            case ',':  return NK_KEY_COMMA;
            case '-':  return NK_KEY_MINUS;
            case '.':  return NK_KEY_PERIOD;
            case '/':  return NK_KEY_SLASH;
            case ';':  return NK_KEY_SEMICOLON;
            case '=':  return NK_KEY_EQUAL;
            case '[':  return NK_KEY_LEFT_BRACKET;
            case '\\': return NK_KEY_BACKSLASH;
            case ']':  return NK_KEY_RIGHT_BRACKET;
            case '`':  return NK_KEY_GRAVE;
            default:   return NK_KEY_UNKNOWN;
        }
    }

    for (size_t i = 0; i < sizeof(key_names) / sizeof(key_names[0]); i++)
    {
        if (name_matches(key_names[i].name, text, length))
        {
            return key_names[i].value;
        }
    }

    return NK_KEY_UNKNOWN;
}

static shortcut_binding_t *find_binding(nk_shortcut_t shortcut)
{
    for (size_t i = 0; i < SHORTCUT_MAX_BINDINGS; i++)
    {
        if (bindings[i].in_use
            && bindings[i].shortcut.key == shortcut.key
            && bindings[i].shortcut.modifiers == shortcut.modifiers)
        {
            return &bindings[i];
        }
    }

    return NULL;
}
