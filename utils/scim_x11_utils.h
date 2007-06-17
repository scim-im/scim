/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * $Id: scim_x11_utils.h,v 1.1 2005/05/11 09:52:11 suzhe Exp $
 */

#ifndef __SCIM_X11_UTILS_H__
#define __SCIM_X11_UTILS_H__

#define Uses_SCIM_EVENT
#include <scim.h>
#include <X11/Xlib.h>

/**
 * @brief Translate a X11 KeyEvent to a scim KeyEvent according to the given Display
 */
scim::KeyEvent scim_x11_keyevent_x11_to_scim (Display *display, const XKeyEvent &xkey);

/**
 * @brief Translate a scim KeyEvent to a X11 KeyEvent according to the given Display
 */
XKeyEvent scim_x11_keyevent_scim_to_x11 (Display *display, const scim::KeyEvent &scimkey);

/**
 * @brief Translate X11 key state to scim key mask.
 */
scim::uint16 scim_x11_keymask_x11_to_scim (Display *display, unsigned int xkeystate);

/**
 * @brief Translate scim key mask to X11 key state.
 */
unsigned int scim_x11_keymask_scim_to_x11 (Display *display, scim::uint16 scimkeymask);

#endif
