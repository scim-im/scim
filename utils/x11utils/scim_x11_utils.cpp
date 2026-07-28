#define Uses_SCIM_EVENT

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <sys/time.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_x11_utils.h"

using namespace scim;

static Time get_time                 (void);
static void initialize_modifier_bits (Display *display);

static Display *__current_display      = 0;
static int      __current_alt_mask     = Mod1Mask;
static int      __current_meta_mask    = 0;
static int      __current_super_mask   = 0;
static int      __current_hyper_mask   = 0;
static int      __current_numlock_mask = Mod2Mask;

static Time
get_time (void)
{
    int tint;
    struct timeval tv;
    struct timezone tz;           /* is not used since ages */
    gettimeofday (&tv, &tz); 
    tint = (int) tv.tv_sec * 1000;
    tint = tint / 1000 * 1000; 
    tint = tint + tv.tv_usec / 1000;
    return ((Time) tint);
}

static void
initialize_modifier_bits (Display *display)
{
    if (__current_display == display)
        return;

    __current_display = display;

    if (display == 0) {
        __current_alt_mask     = Mod1Mask;
        __current_meta_mask    = ShiftMask | Mod1Mask;
        __current_super_mask   = 0;
        __current_hyper_mask   = 0;
        __current_numlock_mask = Mod2Mask;
        return;
    }

    XModifierKeymap *mods;

    ::KeyCode ctrl_l  = XKeysymToKeycode (display, XK_Control_L);
    ::KeyCode ctrl_r  = XKeysymToKeycode (display, XK_Control_R);
    ::KeyCode meta_l  = XKeysymToKeycode (display, XK_Meta_L);
    ::KeyCode meta_r  = XKeysymToKeycode (display, XK_Meta_R);
    ::KeyCode alt_l   = XKeysymToKeycode (display, XK_Alt_L);
    ::KeyCode alt_r   = XKeysymToKeycode (display, XK_Alt_R);
    ::KeyCode super_l = XKeysymToKeycode (display, XK_Super_L);
    ::KeyCode super_r = XKeysymToKeycode (display, XK_Super_R);
    ::KeyCode hyper_l = XKeysymToKeycode (display, XK_Hyper_L);
    ::KeyCode hyper_r = XKeysymToKeycode (display, XK_Hyper_R);
    ::KeyCode numlock = XKeysymToKeycode (display, XK_Num_Lock);

    int i, j;

    mods = XGetModifierMapping (display);

    __current_alt_mask     = 0;
    __current_meta_mask    = 0;
    __current_super_mask   = 0;
    __current_hyper_mask   = 0;
    __current_numlock_mask = 0;

    /* We skip the first three sets for Shift, Lock, and Control.  The
       remaining sets are for Mod1, Mod2, Mod3, Mod4, and Mod5.  */
    for (i = 3; i < 8; i++) {
        for (j = 0; j < mods->max_keypermod; j++) {
            ::KeyCode code = mods->modifiermap [i * mods->max_keypermod + j];
            if (! code) continue;
            if (code == alt_l || code == alt_r)
                __current_alt_mask |= (1 << i);
            else if (code == meta_l || code == meta_r)
                __current_meta_mask |= (1 << i);
            else if (code == super_l || code == super_r)
                __current_super_mask |= (1 << i);
            else if (code == hyper_l || code == hyper_r)
                __current_hyper_mask |= (1 << i);
            else if (code == numlock)
                __current_numlock_mask |= (1 << i);
        }
    }

    /* Check whether there is a combine keys mapped to Meta */
    if (__current_meta_mask == 0) {
        char buf [32];
        XKeyEvent xkey;
        KeySym keysym_l, keysym_r;
 
        xkey.type = KeyPress;
        xkey.display = display;
        xkey.serial = 0L;
        xkey.send_event = False;
        xkey.x = xkey.y = xkey.x_root = xkey.y_root = 0;
        xkey.time = 0;
        xkey.same_screen = False;
        xkey.subwindow = None;
        xkey.window = None;
        xkey.root = DefaultRootWindow (display);
        xkey.state = ShiftMask;

        xkey.keycode = meta_l;
        XLookupString (&xkey, buf, 32, &keysym_l, 0);
        xkey.keycode = meta_r;
        XLookupString (&xkey, buf, 32, &keysym_r, 0);

        if ((meta_l == alt_l && keysym_l == XK_Meta_L) || (meta_r == alt_r && keysym_r == XK_Meta_R))
            __current_meta_mask = ShiftMask + __current_alt_mask;
        else if ((meta_l == ctrl_l && keysym_l == XK_Meta_L) || (meta_r == ctrl_r && keysym_r == XK_Meta_R))
            __current_meta_mask = ShiftMask + ControlMask;
    }

    XFreeModifiermap (mods);
}

KeyEvent
scim_x11_keyevent_x11_to_scim (Display *display, const XKeyEvent &xkey)
{
    KeyEvent  scimkey;
    KeySym    keysym;
    XKeyEvent key = xkey;
    char      buf [32];

    initialize_modifier_bits (display);
 
    XLookupString (&key, buf, 32, &keysym, 0);

    scimkey.code = keysym;

    scimkey.mask = scim_x11_keymask_x11_to_scim (display, xkey.state);

    if (key.type == KeyRelease) scimkey.mask |= SCIM_KEY_ReleaseMask;

    if (scimkey.code == SCIM_KEY_backslash) {
        int keysym_size = 0;
        KeySym *keysyms = XGetKeyboardMapping (display, xkey.keycode, 1, &keysym_size);
        if (keysyms != NULL) {
            if (keysyms[0] == XK_backslash &&
		(keysym_size > 1 && keysyms[1] == XK_underscore))
                scimkey.mask |= SCIM_KEY_QuirkKanaRoMask;
            XFree (keysyms);
        }
    }

    return scimkey;
}

XKeyEvent
scim_x11_keyevent_scim_to_x11 (Display *display, const KeyEvent &scimkey)
{
    XKeyEvent xkey;

    initialize_modifier_bits (display);

    xkey.type = (scimkey.mask & SCIM_KEY_ReleaseMask) ? KeyRelease : KeyPress;
    xkey.display = display;
    xkey.serial = 0L;
    xkey.send_event = False;
    xkey.x = xkey.y = xkey.x_root = xkey.y_root = 0;
    xkey.time = get_time ();
    xkey.same_screen = False;
    xkey.subwindow = None;
    xkey.window = None;

    if (display) {
        xkey.root = DefaultRootWindow (display);
        xkey.keycode = XKeysymToKeycode (display, (KeySym) scimkey.code);
    } else {
        xkey.root = None;
        xkey.keycode = 0;
    }

    xkey.state = scim_x11_keymask_scim_to_x11 (display, scimkey.mask);

    return xkey;
}

uint16
scim_x11_keymask_x11_to_scim (Display *display, unsigned int xkeystate)
{
    uint16 mask = 0;

    initialize_modifier_bits (display);

    // Check Meta mask first, because it's maybe a mask combination.
    if (__current_meta_mask && (xkeystate & __current_meta_mask) == __current_meta_mask) {
        mask |= SCIM_KEY_MetaMask;
        xkeystate &= ~__current_meta_mask;
    }

    if (xkeystate & ShiftMask) {
        mask |= SCIM_KEY_ShiftMask;
        xkeystate &= ~ShiftMask;
    }

    if (xkeystate & LockMask) {
        mask |= SCIM_KEY_CapsLockMask;
        xkeystate &= ~LockMask;
    }

    if (xkeystate & ControlMask) {
        mask |= SCIM_KEY_ControlMask;
        xkeystate &= ~ControlMask;
    }

    if (__current_alt_mask && (xkeystate & __current_alt_mask) == __current_alt_mask) {
        mask |= SCIM_KEY_AltMask;
        xkeystate &= ~__current_alt_mask;
    }

    if (__current_super_mask && (xkeystate & __current_super_mask) == __current_super_mask) {
        mask |= SCIM_KEY_SuperMask;
        xkeystate &= ~__current_super_mask;
    }

    if (__current_hyper_mask && (xkeystate & __current_hyper_mask) == __current_hyper_mask) {
        mask |= SCIM_KEY_HyperMask;
        xkeystate &= ~__current_hyper_mask;
    }

    if (__current_numlock_mask && (xkeystate & __current_numlock_mask) == __current_numlock_mask) {
        mask |= SCIM_KEY_NumLockMask;
    }

    return mask;
}

unsigned int scim_x11_keymask_scim_to_x11 (Display *display, uint16 scimkeymask)
{
    unsigned int state = 0;

    initialize_modifier_bits (display);

    if (scimkeymask & SCIM_KEY_ShiftMask)    state |= ShiftMask;
    if (scimkeymask & SCIM_KEY_CapsLockMask) state |= LockMask;
    if (scimkeymask & SCIM_KEY_ControlMask)  state |= ControlMask;
    if (scimkeymask & SCIM_KEY_AltMask)      state |= __current_alt_mask;
    if (scimkeymask & SCIM_KEY_MetaMask)     state |= __current_meta_mask;
    if (scimkeymask & SCIM_KEY_SuperMask)    state |= __current_super_mask;
    if (scimkeymask & SCIM_KEY_HyperMask)    state |= __current_hyper_mask;
    if (scimkeymask & SCIM_KEY_NumLockMask)  state |= __current_numlock_mask;

    return state;
}
