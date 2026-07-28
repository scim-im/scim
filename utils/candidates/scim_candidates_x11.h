/**
 * @file scim_candidates_x11.h
 * @brief X11 surface shim for the Cairo input-panel renderer.
 *
 * Owns an override-redirect toplevel + a cairo-xlib surface, positions it at
 * an absolute spot location with XMoveWindow, pumps its own X events and routes
 * pointer input back through CandidatesUI::hit_test(). This is the X11 half of the
 * "own-Cairo" candidate UI; the Wayland shim (5c) will mirror it against an
 * input-popup-surface-v2.
 *
 * The shim opens its own Display connection to the running X server so it does
 * not share Xlib state with the XIM frontend on the same daemon.
 */

/*
 * Smart Common Input Method
 *
 * Copyright (c) 2026 SCIM developers
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
 */

#pragma once

#include <scim.h>
#include <functional>
#include "scim_candidates.h"

namespace scim {

/**
 * @brief X11 override-redirect surface driving a CandidatesUI renderer.
 */
class CandidatesUIX11
{
    class CandidatesUIX11Impl;
    CandidatesUIX11Impl *m_impl;

    CandidatesUIX11 (const CandidatesUIX11 &);
    const CandidatesUIX11 & operator = (const CandidatesUIX11 &);

public:
    /** @brief Callback when a candidate is clicked (page-relative index). */
    typedef std::function<void (int)> CandidateSlot;
    /** @brief Callback for a page-flip request (scroll wheel). */
    typedef std::function<void ()>    PageSlot;

    CandidatesUIX11 ();
    ~CandidatesUIX11 ();

    /**
     * @brief Open a connection and create the panel window.
     * @param display_name  X display, empty for $DISPLAY.
     * @return true on success.
     */
    bool open  (const String &display_name = String ());
    void close ();
    bool is_open () const;

    /** @brief Access the renderer to push preedit/aux/lookup state. */
    CandidatesUI & ui ();

    /** @brief Xlib connection fd, for the caller's poll/select loop (-1 if closed). */
    int  connection_number () const;
    /** @brief Drain and handle pending X events for the panel window. */
    void process_events ();

    /** @brief Set the anchor (absolute root coords of the text cursor). */
    void move   (int x, int y);
    /** @brief Re-measure, resize and repaint from current renderer state. */
    void update ();
    /** @brief Map (show) the window. */
    void show   ();
    /** @brief Unmap (hide) the window. */
    void hide   ();
    /** @brief Whether the window is currently mapped. */
    bool is_shown () const;

    /** @name Pointer callbacks @{ */
    void signal_connect_candidate_selected (CandidateSlot slot);
    void signal_connect_page_up            (PageSlot slot);
    void signal_connect_page_down          (PageSlot slot);
    /** @} */
};

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
