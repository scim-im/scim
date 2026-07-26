/**
 * @file scim_panel_ui_wayland.h
 * @brief Wayland surface shim for the Cairo input-panel renderer.
 *
 * Owns a wl_surface used as a zwp_input_popup_surface_v2 (created from the
 * frontend's zwp_input_method_v2 object) plus an shm buffer that PanelUI draws
 * into. The compositor positions the popup at the text cursor; we only supply
 * content and route wl_pointer input back through PanelUI::hit_test(). This is
 * the Wayland counterpart of PanelUIX11.
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

#include <functional>
#include "scim_panel_ui.h"

struct wl_display;
struct wl_compositor;
struct wl_shm;
struct wl_seat;
struct zwp_input_method_v2;

namespace scim {

class PanelUIWaylandImpl;   // defined in scim_panel_ui_wayland.cpp

/**
 * @brief input-popup-surface-v2 backend driving a PanelUI renderer.
 */
class PanelUIWayland
{
    PanelUIWaylandImpl *m_impl;

    PanelUIWayland (const PanelUIWayland &);
    const PanelUIWayland & operator = (const PanelUIWayland &);

public:
    /** @brief Callback when a candidate is clicked (page-relative index). */
    typedef std::function<void (int)> CandidateSlot;
    /** @brief Callback for a page-flip request (scroll wheel). */
    typedef std::function<void ()>    PageSlot;

    PanelUIWayland ();
    ~PanelUIWayland ();

    /**
     * @brief Create the popup surface from the frontend's Wayland objects.
     * @param display     the shared wl_display (for flushing).
     * @param compositor  wl_compositor global.
     * @param shm         wl_shm global.
     * @param input_method the zwp_input_method_v2 this popup belongs to.
     * @param seat        wl_seat, for pointer input (may be null).
     * @return true if the popup surface was created.
     */
    bool init (struct wl_display *display,
               struct wl_compositor *compositor,
               struct wl_shm *shm,
               struct zwp_input_method_v2 *input_method,
               struct wl_seat *seat);

    /** @brief Tear down the popup surface and buffers. */
    void finish ();
    /** @brief Whether the popup surface exists. */
    bool is_ready () const;

    /** @brief Access the renderer to push aux/lookup state. */
    PanelUI & ui ();

    /**
     * @brief Re-measure, redraw and commit; or blank the surface when the
     *        renderer has nothing visible. Call after any state change.
     */
    void update ();
    /** @brief Draw current state (same as update()). */
    void show ();
    /** @brief Blank the surface (attach a null buffer). */
    void hide ();

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
