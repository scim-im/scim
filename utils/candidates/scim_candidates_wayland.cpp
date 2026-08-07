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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define Uses_SCIM_DEBUG
#include "scim_private.h"
#include <scim.h>

#include "scim_candidates_wayland.h"

#include <wayland-client.h>
#include "input-method-unstable-v1-client-protocol.h"
#include "input-method-unstable-v2-client-protocol.h"

#include <cairo.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <linux/input-event-codes.h>

namespace scim {

namespace {

// A throwaway shm buffer: freed when the compositor releases it, or by the
// owner at teardown for the one still attached, which is never released because
// the surface it was attached to has gone.
struct ShmBuffer {
    struct wl_buffer *buffer;
    void             *data;
    size_t            size;
    int               width;
    int               height;
    int               stride;
};

void free_shm_buffer (ShmBuffer *b)
{
    if (b->data)   munmap (b->data, b->size);
    if (b->buffer) wl_buffer_destroy (b->buffer);
    delete b;
}

} // anonymous namespace

class CandidatesWaylandImpl
{
public:
    CandidatesUI          m_ui;

    struct wl_display    *m_display;
    struct wl_compositor *m_compositor;
    struct wl_shm        *m_shm;
    struct zwp_input_method_v2       *m_input_method;
    struct wl_seat       *m_seat;

    struct wl_surface    *m_surface;
    // Exactly one role is set, depending on which init() was used.
    struct zwp_input_popup_surface_v2 *m_popup;
    struct zwp_input_panel_surface_v1 *m_panel_surface;
    struct wl_pointer    *m_pointer;

    // Set from the input-popup surface's text_input_rectangle: true when the
    // compositor placed us above the text rather than below it.
    bool     m_text_below;

    // Whether the compositor has activated the input method, i.e. whether there
    // is a focused text input for this popup to be positioned against. Nothing is
    // committed to the surface until it is.
    bool     m_active;
    // Whether a buffer is currently attached, so an unmap is committed once
    // rather than on every update that finds nothing to show.
    bool     m_mapped;

    // Buffer scale. The surface is laid out in logical pixels and the buffer is
    // rendered at this multiple of them, so the panel is sharp on a HiDPI output
    // instead of being a 1x buffer the compositor blows up.
    //
    // m_preferred_scale comes from wl_surface.preferred_buffer_scale, which is
    // per surface and exactly what the compositor wants; it needs wl_compositor
    // version 6 (wayland 1.22). m_output_scale is the fallback for older
    // compositors: the largest scale among the outputs, which is safe wherever
    // the popup ends up -- set_buffer_scale states what the buffer is a multiple
    // of, so the logical size stays right and an over-scaled buffer is merely
    // downscaled on a 1x output.
    int      m_preferred_scale;
    int      m_output_scale;

    // Our own registry, used only to watch outputs for the fallback above. The
    // frontend has one of its own; a second is independent and costs a handful of
    // events at startup, which is cheaper than plumbing output tracking through
    // the frontend for a number only this file uses.
    struct wl_registry *m_registry;
    std::vector<struct wl_output *> m_outputs;

    // Pointer state.
    bool     m_pointer_on_surface;
    double   m_ptr_x, m_ptr_y;
    uint32_t m_enter_serial;

    // Scroll state. wl_pointer reports an axis in three ways and a client is
    // meant to apply them at the frame that closes the batch, not as they
    // arrive: value120 in 120ths of a notch (v8), axis_discrete in whole
    // notches (v5), and axis as a continuous length. Paging on each axis event
    // instead ran a touchpad swipe through a page per report.
    //
    // Only what the bound wl_pointer version sends is ever seen -- the seat is
    // bound at 7 today, so discrete arrives and value120 does not -- but
    // handling both means raising that binding needs no change here.
    double   m_axis_frame;        // continuous units this frame
    int      m_discrete_frame;    // whole notches this frame
    int      m_value120_frame;    // 120ths of a notch this frame
    double   m_axis_accum;        // continuous remainder carried between frames
    int      m_value120_accum;    // 120ths remainder carried between frames

    void reset_scroll ()
    {
        m_axis_frame = 0.0;
        m_discrete_frame = 0;
        m_value120_frame = 0;
        m_axis_accum = 0.0;
        m_value120_accum = 0;
    }

    int scale () const
    {
        int s = m_preferred_scale > 0 ? m_preferred_scale : m_output_scale;
        return s > 0 ? s : 1;
    }

    /** @brief Start following the scale; call once the surface exists. */
    void watch_scale ();

    /**
     * @brief Start following the seat's pointer capability.
     *
     * The pointer is not taken up front. wl_seat.get_pointer on a seat that has
     * never had the pointer capability is the missing_capability protocol error,
     * and a protocol error does not merely fail the request -- the compositor
     * drops the connection, taking the input method down with it. A seat with no
     * pointer is unusual on a desktop and ordinary on a tablet, a kiosk or a
     * headless session, so this waits to be told.
     *
     * The seat listener is owned here; nothing else in the process may add one
     * to the same proxy, since libwayland allows a proxy only one.
     */
    void watch_seat ();

    /** @brief Create or drop the wl_pointer as the capability comes and goes. */
    void set_pointer_capability (bool have_pointer);

    /** @brief Release the pointer, by the request its version actually has. */
    void destroy_pointer ();

    CandidatesWayland::CandidateSlot m_candidate_slot;
    CandidatesWayland::PageSlot      m_page_up_slot;
    CandidatesWayland::PageSlot      m_page_down_slot;

    CandidatesWaylandImpl ()
        : m_display (0), m_compositor (0), m_shm (0), m_input_method (0),
          m_seat (0), m_surface (0), m_popup (0), m_panel_surface (0), m_pointer (0),
          m_text_below (false),
          m_active (false), m_mapped (false),
          m_preferred_scale (0), m_output_scale (0), m_registry (0),
          m_pointer_on_surface (false), m_ptr_x (0), m_ptr_y (0),
          m_enter_serial (0),
          m_axis_frame (0.0), m_discrete_frame (0), m_value120_frame (0),
          m_axis_accum (0.0), m_value120_accum (0)
    {
    }

    ~CandidatesWaylandImpl ()
    {
        finish ();
    }

    void finish ()
    {
        destroy_pointer ();
        if (m_popup)   { zwp_input_popup_surface_v2_destroy (m_popup); m_popup = 0; }
        if (m_panel_surface) {
            zwp_input_panel_surface_v1_destroy (m_panel_surface);
            m_panel_surface = 0;
        }
        if (m_surface) { wl_surface_destroy (m_surface); m_surface = 0; }
        // After the surface, so nothing is drawing from them any more. The
        // buffer still attached at this point is the one whose release event
        // will never come, and freeing it here is the only thing that ever
        // unmaps it.
        destroy_live_buffers ();
        for (size_t i = 0; i < m_outputs.size (); ++i)
            wl_output_destroy (m_outputs[i]);
        m_outputs.clear ();
        if (m_registry) { wl_registry_destroy (m_registry); m_registry = 0; }
    }

    /** @brief Allocate an shm buffer and record it in m_live_buffers. */
    ShmBuffer * create_buffer (int w, int h);
    /** @brief Unmap and destroy one buffer, and drop it from m_live_buffers. */
    void free_buffer (ShmBuffer *b);
    /** @brief Unmap and destroy every buffer still outstanding. */
    void destroy_live_buffers ();
    /** @brief wl_buffer.release trampoline. */
    static void handle_buffer_release (void *data, struct wl_buffer *);

    // Buffers handed to the compositor and not released yet. Normally at most
    // one or two; tracked so teardown can free them, and so a compositor that
    // stops sending release leaks visibly here rather than invisibly.
    std::vector<ShmBuffer *> m_live_buffers;

    void blank ()
    {
        // Drop a partial notch with the list it belonged to.
        reset_scroll ();
        if (!m_surface || !m_mapped) return;
        wl_surface_attach (m_surface, 0, 0, 0);
        wl_surface_commit (m_surface);
        m_mapped = false;
        if (m_display) wl_display_flush (m_display);
    }

    // Called when the compositor activates or deactivates the input method.
    void set_active (bool active)
    {
        if (m_active == active)
            return;
        m_active = active;
        if (m_active) update ();   // there may already be something to show
        else          blank ();
    }

    void update ()
    {
        if (!m_surface || !m_shm)
            return;

        // Commit nothing until the input method is active. An input popup is
        // positioned relative to the focused text input's surface, so without one
        // there is nowhere for this to go -- and sway dereferences the focused
        // surface's scene node when it processes the commit, which segfaults it
        // when no such surface is current (input_popup_update () ->
        // wlr_scene_node_coords ()). Staying unmapped until then is also simply
        // the truth: there is nothing to draw candidates next to.
        if (!m_active) {
            blank ();
            return;
        }

        if (!m_ui.is_visible ()) {
            blank ();
            return;
        }

        // The renderer works in logical pixels throughout; only the buffer is
        // bigger. Buffer dimensions therefore come out an exact multiple of the
        // scale, which set_buffer_scale requires.
        const int s = scale ();

        int w = 0, h = 0;
        m_ui.measure (w, h);

        ShmBuffer *b = create_buffer (w * s, h * s);
        if (!b) {
            blank ();
            return;
        }

        cairo_surface_t *cs = cairo_image_surface_create_for_data (
            static_cast<unsigned char *> (b->data), CAIRO_FORMAT_ARGB32,
            b->width, b->height, b->stride);
        cairo_t *cr = cairo_create (cs);
        if (s != 1)
            cairo_scale (cr, s, s);
        m_ui.draw (cr);
        cairo_destroy (cr);
        cairo_surface_destroy (cs);

        // Unconditional, not just when scaling up: the scale is surface state
        // that persists across commits, so coming back down to 1 has to be said
        // too or the surface keeps the old factor and maps at a fraction of its
        // logical size. Guarded only on the version that has the request.
        if (wl_proxy_get_version ((struct wl_proxy *) m_surface) >= 3)
            wl_surface_set_buffer_scale (m_surface, s);

        // TODO(5c): a per-frame throwaway shm buffer is simple but allocates
        // each update; reuse a buffer pool once the update rate matters.
        wl_surface_attach (m_surface, b->buffer, 0, 0);
        wl_surface_damage_buffer (m_surface, 0, 0, b->width, b->height);
        wl_surface_commit (m_surface);
        m_mapped = true;
        if (m_display) wl_display_flush (m_display);
    }

    void handle_button (uint32_t button, uint32_t state)
    {
        if (state != WL_POINTER_BUTTON_STATE_PRESSED)
            return;
        if (button != BTN_LEFT)
            return;
        int idx = -1;
        CandidatesUI::HitType hit =
            m_ui.hit_test (static_cast<int> (m_ptr_x),
                           static_cast<int> (m_ptr_y), idx);
        if (hit == CandidatesUI::HIT_CANDIDATE && idx >= 0) {
            if (m_candidate_slot) m_candidate_slot (idx);
        } else if (hit == CandidatesUI::HIT_PREV_PAGE) {
            if (m_page_up_slot) m_page_up_slot ();
        } else if (hit == CandidatesUI::HIT_NEXT_PAGE) {
            if (m_page_down_slot) m_page_down_slot ();
        }
    }

    // A wheel notch in continuous axis units. The protocol fixes no value;
    // compositors emit 10 or 15 per detent, so 10 pages on the smaller of the
    // two rather than needing one and a half notches on the larger.
    static constexpr double AXIS_STEP = 10.0;

    void handle_axis (uint32_t axis, wl_fixed_t value)
    {
        if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL)
            return;
        m_axis_frame += wl_fixed_to_double (value);

        // wl_pointer.frame arrives at version 5, along with the discrete and
        // stop events. Below that nothing would ever close the batch and the
        // axis would accumulate unapplied, so the report is the whole batch.
        if (!m_pointer ||
            wl_proxy_get_version ((struct wl_proxy *) m_pointer)
                < WL_POINTER_FRAME_SINCE_VERSION)
            handle_frame ();
    }

    void handle_axis_discrete (uint32_t axis, int32_t discrete)
    {
        if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL)
            return;
        m_discrete_frame += discrete;
    }

    void handle_axis_value120 (uint32_t axis, int32_t value120)
    {
        if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL)
            return;
        m_value120_frame += value120;
    }

    void page_by (int steps)
    {
        // Bounded: a fling can report a large count, and each page is a round
        // trip to the engine.
        const int max_steps = 5;
        if (steps >  max_steps) steps =  max_steps;
        if (steps < -max_steps) steps = -max_steps;

        for (int i = steps; i < 0; ++ i)
            if (m_page_up_slot) m_page_up_slot ();
        for (int i = 0; i < steps; ++ i)
            if (m_page_down_slot) m_page_down_slot ();
    }

    // The batch is complete: turn whichever of the three the compositor sent
    // into pages. Most precise first, so a device reporting both is counted
    // once.
    void handle_frame ()
    {
        int steps = 0;

        if (m_value120_frame) {
            if ((m_value120_frame < 0) != (m_value120_accum < 0))
                m_value120_accum = 0;
            m_value120_accum += m_value120_frame;
            steps = m_value120_accum / 120;
            m_value120_accum -= steps * 120;
        } else if (m_discrete_frame) {
            steps = m_discrete_frame;
        } else if (m_axis_frame != 0.0) {
            if ((m_axis_frame < 0.0) != (m_axis_accum < 0.0))
                m_axis_accum = 0.0;
            m_axis_accum += m_axis_frame;
            steps = (int) (m_axis_accum / AXIS_STEP);
            m_axis_accum -= steps * AXIS_STEP;
        }

        m_axis_frame = 0.0;
        m_discrete_frame = 0;
        m_value120_frame = 0;

        if (steps)
            page_by (steps);
    }
};

/* ------------------------------------------------------------------ */
/* Pointer listener                                                    */
/* ------------------------------------------------------------------ */

namespace {

void ptr_enter (void *data, struct wl_pointer *, uint32_t serial,
                struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (surface == d->m_surface) {
        d->m_pointer_on_surface = true;
        d->m_enter_serial = serial;
        d->m_ptr_x = wl_fixed_to_double (sx);
        d->m_ptr_y = wl_fixed_to_double (sy);
    }
}

void ptr_leave (void *data, struct wl_pointer *, uint32_t,
                struct wl_surface *surface)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (surface == d->m_surface) {
        d->m_pointer_on_surface = false;
        d->reset_scroll ();
    }
}

void ptr_motion (void *data, struct wl_pointer *, uint32_t,
                 wl_fixed_t sx, wl_fixed_t sy)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface) {
        d->m_ptr_x = wl_fixed_to_double (sx);
        d->m_ptr_y = wl_fixed_to_double (sy);
    }
}

void ptr_button (void *data, struct wl_pointer *, uint32_t, uint32_t,
                 uint32_t button, uint32_t state)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface)
        d->handle_button (button, state);
}

void ptr_axis (void *data, struct wl_pointer *, uint32_t, uint32_t axis,
               wl_fixed_t value)
{
    CandidatesWaylandImpl *d =
        static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface)
        d->handle_axis (axis, value);
}

void ptr_frame (void *data, struct wl_pointer *)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface)
        d->handle_frame ();
}

void ptr_axis_source (void *, struct wl_pointer *, uint32_t) {}

void ptr_axis_stop (void *data, struct wl_pointer *, uint32_t, uint32_t axis)
{
    // The gesture ended: what did not add up to a notch never will.
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        d->m_axis_accum = 0.0;
        d->m_value120_accum = 0;
    }
}

void ptr_axis_discrete (void *data, struct wl_pointer *, uint32_t axis,
                        int32_t discrete)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface)
        d->handle_axis_discrete (axis, discrete);
}

void ptr_axis_value120 (void *data, struct wl_pointer *, uint32_t axis,
                        int32_t value120)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    if (d->m_pointer_on_surface)
        d->handle_axis_value120 (axis, value120);
}
void ptr_axis_relative_direction (void *, struct wl_pointer *, uint32_t, uint32_t) {}

const struct wl_pointer_listener pointer_listener = {
    ptr_enter,
    ptr_leave,
    ptr_motion,
    ptr_button,
    ptr_axis,
    ptr_frame,
    ptr_axis_source,
    ptr_axis_stop,
    ptr_axis_discrete,
    ptr_axis_value120,
    ptr_axis_relative_direction,
};

/* ------------------------------------------------------------------ */
/* Seat capabilities                                                   */
/* ------------------------------------------------------------------ */

void seat_capabilities (void *data, struct wl_seat *, uint32_t caps)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    d->set_pointer_capability ((caps & WL_SEAT_CAPABILITY_POINTER) != 0);
}

void seat_name (void *, struct wl_seat *, const char *) { }

const struct wl_seat_listener seat_listener = {
    seat_capabilities,
    seat_name,
};

// input-popup-surface: the compositor reports the text input area as a rectangle
// in *our* surface coordinates. It cannot be used to position the panel -- the
// compositor owns placement, and the surface is sized exactly to the content, so
// there is nothing to align within and nothing to move. What it does tell us is
// which side of the panel the text sits on: a positive y means the text is below
// us, i.e. the compositor put the panel above the text (typically near the
// bottom of the screen). In that case mirror the section order so the block
// nearest the text is the one closest to it.
//
// input-method-v1 has no equivalent event, so this refinement is v2 only; there
// the section order simply stays as laid out.
void popup_text_input_rectangle (void *data, struct zwp_input_popup_surface_v2 *,
                                 int32_t, int32_t y, int32_t, int32_t)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    bool text_below = (y > 0);
    if (d->m_text_below == text_below)
        return;
    d->m_text_below = text_below;
    d->m_ui.set_sections_reversed (text_below);
    // Re-render only when something is already on screen; otherwise the next
    // update () picks the new order up.
    if (d->m_ui.is_visible ())
        d->update ();
}

const struct zwp_input_popup_surface_v2_listener popup_listener = {
    popup_text_input_rectangle,
};

/* ------------------------------------------------------------------ */
/* Buffer scale                                                        */
/* ------------------------------------------------------------------ */

// Redraw at the new scale if something is on screen; otherwise the next
// update () picks it up.
void rescaled (CandidatesWaylandImpl *d, int before)
{
    if (d->scale () != before && d->m_ui.is_visible ())
        d->update ();
}

void surface_enter (void *, struct wl_surface *, struct wl_output *) { }
void surface_leave (void *, struct wl_surface *, struct wl_output *) { }

#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
void surface_preferred_buffer_scale (void *data, struct wl_surface *,
                                     int32_t factor)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    int before = d->scale ();
    d->m_preferred_scale = factor > 0 ? factor : 1;
    rescaled (d, before);
}

void surface_preferred_buffer_transform (void *, struct wl_surface *, uint32_t) { }
#endif

// As long as the headers this was built against make it, and libwayland
// dispatches an event by indexing it with the opcode without checking the
// length. The surface must therefore never be bound above the version these
// same headers describe -- see where the frontend binds wl_compositor.
const struct wl_surface_listener surface_listener = {
    surface_enter,
    surface_leave,
#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
    surface_preferred_buffer_scale,
    surface_preferred_buffer_transform,
#endif
};

// Fallback for compositors without wl_surface.preferred_buffer_scale: track the
// largest scale any output reports. Not which output we are on -- an input popup
// is placed by the compositor and never told where it landed -- so this
// deliberately over-scales on the smaller output of a mixed-DPI pair rather than
// under-scaling on the larger one. It only ever rises, so unplugging the HiDPI
// monitor of such a pair leaves it high until restart; that costs a larger
// buffer for a small window, never a wrongly sized one.
void output_scale (void *data, struct wl_output *, int32_t factor)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    if (factor <= 0)
        return;
    int before = d->scale ();
    if (factor > d->m_output_scale)
        d->m_output_scale = factor;
    rescaled (d, before);
}

void output_geometry (void *, struct wl_output *, int32_t, int32_t, int32_t,
                      int32_t, int32_t, const char *, const char *, int32_t) { }
void output_mode (void *, struct wl_output *, uint32_t, int32_t, int32_t,
                  int32_t) { }
void output_done (void *, struct wl_output *) { }
void output_name (void *, struct wl_output *, const char *) { }
void output_description (void *, struct wl_output *, const char *) { }

const struct wl_output_listener output_listener = {
    output_geometry,
    output_mode,
    output_done,
    output_scale,
#ifdef WL_OUTPUT_NAME_SINCE_VERSION
    output_name,
    output_description,
#endif
};

void registry_global (void *data, struct wl_registry *registry, uint32_t name,
                      const char *interface, uint32_t version)
{
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    if (strcmp (interface, wl_output_interface.name))
        return;
    // Version 2 is where wl_output.scale arrives, and all this registry is for.
    uint32_t want = 2;
    if (want > (uint32_t) wl_output_interface.version)
        want = (uint32_t) wl_output_interface.version;
    if (version < want)
        return;
    struct wl_output *output = static_cast<struct wl_output *> (
        wl_registry_bind (registry, name, &wl_output_interface, want));
    if (!output)
        return;
    wl_output_add_listener (output, &output_listener, d);
    d->m_outputs.push_back (output);
}

void registry_global_remove (void *, struct wl_registry *, uint32_t) { }

const struct wl_registry_listener registry_listener = {
    registry_global,
    registry_global_remove,
};

const struct wl_buffer_listener buffer_listener = {
    CandidatesWaylandImpl::handle_buffer_release,
};

} // anonymous namespace

/* ------------------------------------------------------------------ */
/* Buffers                                                             */
/* ------------------------------------------------------------------ */

ShmBuffer *
CandidatesWaylandImpl::create_buffer (int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    int stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, w);
    size_t size = static_cast<size_t> (stride) * h;

    int fd = memfd_create ("scim-panel", MFD_CLOEXEC);
    if (fd < 0)
        return 0;
    if (ftruncate (fd, size) < 0) {
        close (fd);
        return 0;
    }

    void *data = mmap (0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close (fd);
        return 0;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool (m_shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer (
        pool, 0, w, h, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy (pool);
    close (fd);

    ShmBuffer *b = new ShmBuffer;
    b->buffer = buffer;
    b->data   = data;
    b->size   = size;
    b->width  = w;
    b->height = h;
    b->stride = stride;
    m_live_buffers.push_back (b);
    wl_buffer_add_listener (buffer, &buffer_listener, this);
    return b;
}

void
CandidatesWaylandImpl::free_buffer (ShmBuffer *b)
{
    if (!b)
        return;
    for (size_t i = 0; i < m_live_buffers.size (); ++i) {
        if (m_live_buffers[i] == b) {
            m_live_buffers.erase (m_live_buffers.begin () + i);
            break;
        }
    }
    free_shm_buffer (b);
}

void
CandidatesWaylandImpl::destroy_live_buffers ()
{
    // Emptied first, so nothing walks a vector that free_shm_buffer () would
    // otherwise be unlinking from underneath it.
    std::vector<ShmBuffer *> live;
    live.swap (m_live_buffers);
    for (size_t i = 0; i < live.size (); ++i)
        free_shm_buffer (live[i]);
}

void
CandidatesWaylandImpl::handle_buffer_release (void *data, struct wl_buffer *buffer)
{
    // The listener carries the impl rather than the buffer, so that a release
    // arriving after the buffer was already freed at teardown cannot reach a
    // dangling ShmBuffer. The proxy identifies which one.
    CandidatesWaylandImpl *d = static_cast<CandidatesWaylandImpl *> (data);
    for (size_t i = 0; i < d->m_live_buffers.size (); ++i) {
        if (d->m_live_buffers[i]->buffer == buffer) {
            d->free_buffer (d->m_live_buffers[i]);
            return;
        }
    }
}

void
CandidatesWaylandImpl::destroy_pointer ()
{
    if (!m_pointer)
        return;

    // release is a destructor request and arrives at wl_pointer version 3;
    // sending it to an older proxy is a fatal protocol error, so fall back to
    // dropping the proxy locally there, as the pre-v3 protocol expects.
    if (wl_proxy_get_version ((struct wl_proxy *) m_pointer)
            >= WL_POINTER_RELEASE_SINCE_VERSION)
        wl_pointer_release (m_pointer);
    else
        wl_pointer_destroy (m_pointer);

    m_pointer = 0;
    m_pointer_on_surface = false;
    reset_scroll ();
}

void
CandidatesWaylandImpl::set_pointer_capability (bool have_pointer)
{
    if (have_pointer == (m_pointer != 0))
        return;

    if (!have_pointer) {
        destroy_pointer ();
        return;
    }

    if (!m_seat)
        return;

    m_pointer = wl_seat_get_pointer (m_seat);
    if (m_pointer)
        wl_pointer_add_listener (m_pointer, &pointer_listener, this);
}

void
CandidatesWaylandImpl::watch_seat ()
{
    if (!m_seat)
        return;

    // Nothing happens until the compositor answers with capabilities; clicking a
    // candidate simply does not work until then, which is a fraction of a second
    // at startup and correct on a seat that never gains a pointer.
    if (wl_seat_add_listener (m_seat, &seat_listener, this) < 0)
        SCIM_DEBUG_FRONTEND (1) << "candidates -- the seat already has a "
                                   "listener; pointer input is unavailable.\n";
}

void
CandidatesWaylandImpl::watch_scale ()
{
    if (!m_surface)
        return;

    // Fires only where the compositor offers wl_surface version 6; below that
    // the surface simply never reports a preference and the outputs answer
    // instead.
    wl_surface_add_listener (m_surface, &surface_listener, this);

    if (!m_display || m_registry)
        return;
    m_registry = wl_display_get_registry (m_display);
    if (m_registry)
        wl_registry_add_listener (m_registry, &registry_listener, this);
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

CandidatesWayland::CandidatesWayland ()
    : m_impl (new CandidatesWaylandImpl ())
{
}

CandidatesWayland::~CandidatesWayland ()
{
    delete m_impl;
}

bool
CandidatesWayland::init (struct wl_display *display,
                      struct wl_compositor *compositor,
                      struct wl_shm *shm,
                      struct zwp_input_method_v2 *input_method,
                      struct wl_seat *seat)
{
    CandidatesWaylandImpl *d = m_impl;
    if (!display || !compositor || !shm || !input_method)
        return false;

    d->m_display      = display;
    d->m_compositor   = compositor;
    d->m_shm          = shm;
    d->m_input_method = input_method;
    d->m_seat         = seat;

    d->m_surface = wl_compositor_create_surface (compositor);
    if (!d->m_surface)
        return false;

    d->m_popup = zwp_input_method_v2_get_input_popup_surface (
        input_method, d->m_surface);
    if (!d->m_popup) {
        wl_surface_destroy (d->m_surface);
        d->m_surface = 0;
        return false;
    }
    zwp_input_popup_surface_v2_add_listener (d->m_popup, &popup_listener, d);

    d->watch_scale ();
    d->watch_seat ();

    return true;
}

bool
CandidatesWayland::init_input_panel (struct wl_display *display,
                                     struct wl_compositor *compositor,
                                     struct wl_shm *shm,
                                     struct zwp_input_panel_v1 *panel,
                                     struct wl_seat *seat)
{
    CandidatesWaylandImpl *d = m_impl;
    if (!display || !compositor || !shm || !panel)
        return false;

    d->m_display    = display;
    d->m_compositor = compositor;
    d->m_shm        = shm;
    d->m_seat       = seat;

    d->m_surface = wl_compositor_create_surface (compositor);
    if (!d->m_surface)
        return false;

    d->m_panel_surface =
        zwp_input_panel_v1_get_input_panel_surface (panel, d->m_surface);
    if (!d->m_panel_surface) {
        wl_surface_destroy (d->m_surface);
        d->m_surface = 0;
        return false;
    }

    // Overlay panel rather than set_toplevel (): a candidate window follows the
    // text being typed, so it should float over the app instead of being docked
    // as a full-width on-screen keyboard. zwp_input_panel_surface_v1 has no
    // listener -- unlike the v2 popup there is no cursor-rectangle event, so the
    // compositor's placement is all we get.
    zwp_input_panel_surface_v1_set_overlay_panel (d->m_panel_surface);

    d->watch_scale ();
    d->watch_seat ();

    return true;
}

void
CandidatesWayland::finish ()
{
    m_impl->finish ();
}

bool
CandidatesWayland::is_ready () const
{
    return m_impl->m_popup != 0 || m_impl->m_panel_surface != 0;
}

CandidatesUI &
CandidatesWayland::ui ()
{
    return m_impl->m_ui;
}

void
CandidatesWayland::update ()
{
    m_impl->update ();
}

void
CandidatesWayland::show ()
{
    m_impl->update ();
}

void
CandidatesWayland::hide ()
{
    m_impl->blank ();
}

void
CandidatesWayland::set_active (bool active)
{
    m_impl->set_active (active);
}

void
CandidatesWayland::signal_connect_select_candidate (CandidateSlot slot)
{
    m_impl->m_candidate_slot = slot;
}

void
CandidatesWayland::signal_connect_page_up (PageSlot slot)
{
    m_impl->m_page_up_slot = slot;
}

void
CandidatesWayland::signal_connect_page_down (PageSlot slot)
{
    m_impl->m_page_down_slot = slot;
}

/* ------------------------------------------------------------------ */
/* CandidatesSink                                                      */
/* ------------------------------------------------------------------ */

void
CandidatesWayland::enable (bool enabled)
{
    if (!is_ready () || enabled)
        return;
    m_impl->m_ui.hide_preedit_string ();
    m_impl->m_ui.hide_aux_string ();
    m_impl->m_ui.hide_lookup_table ();
    hide ();
}

void
CandidatesWayland::update_preedit_string (const WideString &str,
                                          const AttributeList &attrs)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_preedit_string (str, attrs);
    update ();
}

void
CandidatesWayland::update_preedit_caret (int caret)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_preedit_caret (caret);
    update ();
}

void
CandidatesWayland::show_preedit_string (bool visible)
{
    if (!is_ready ()) return;
    if (visible) m_impl->m_ui.show_preedit_string ();
    else         m_impl->m_ui.hide_preedit_string ();
    update ();
}

void
CandidatesWayland::update_aux_string (const WideString &str,
                                      const AttributeList &attrs)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_aux_string (str, attrs);
    update ();
}

void
CandidatesWayland::show_aux_string (bool visible)
{
    if (!is_ready ()) return;
    if (visible) m_impl->m_ui.show_aux_string ();
    else         m_impl->m_ui.hide_aux_string ();
    update ();
}

void
CandidatesWayland::update_lookup_table (const LookupTable &table)
{
    if (!is_ready ()) return;
    m_impl->m_ui.update_lookup_table (table);
    update ();
}

void
CandidatesWayland::show_lookup_table (bool visible)
{
    if (!is_ready ()) return;
    if (visible) m_impl->m_ui.show_lookup_table ();
    else         m_impl->m_ui.hide_lookup_table ();
    update ();
}

void
CandidatesWayland::update_spot_location (int x, int y)
{
    (void) x;
    (void) y;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
