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
#include "input-method-unstable-v2-client-protocol.h"

#include <cairo.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <linux/input-event-codes.h>

namespace scim {

namespace {

// A throwaway shm buffer: freed when the compositor releases it.
struct ShmBuffer {
    struct wl_buffer *buffer;
    void             *data;
    size_t            size;
    int               width;
    int               height;
    int               stride;
};

const struct wl_buffer_listener buffer_listener = {
    // release
    [] (void *data, struct wl_buffer *) {
        ShmBuffer *b = static_cast<ShmBuffer *> (data);
        if (b->data) munmap (b->data, b->size);
        if (b->buffer) wl_buffer_destroy (b->buffer);
        delete b;
    }
};

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
    struct zwp_input_popup_surface_v2 *m_popup;
    struct wl_pointer    *m_pointer;

    // Pointer state.
    bool     m_pointer_on_surface;
    double   m_ptr_x, m_ptr_y;
    uint32_t m_enter_serial;

    CandidatesWayland::CandidateSlot m_candidate_slot;
    CandidatesWayland::PageSlot      m_page_up_slot;
    CandidatesWayland::PageSlot      m_page_down_slot;

    CandidatesWaylandImpl ()
        : m_display (0), m_compositor (0), m_shm (0), m_input_method (0),
          m_seat (0), m_surface (0), m_popup (0), m_pointer (0),
          m_pointer_on_surface (false), m_ptr_x (0), m_ptr_y (0),
          m_enter_serial (0)
    {
    }

    ~CandidatesWaylandImpl ()
    {
        finish ();
    }

    void finish ()
    {
        if (m_pointer) { wl_pointer_destroy (m_pointer); m_pointer = 0; }
        if (m_popup)   { zwp_input_popup_surface_v2_destroy (m_popup); m_popup = 0; }
        if (m_surface) { wl_surface_destroy (m_surface); m_surface = 0; }
    }

    ShmBuffer * create_buffer (int w, int h)
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
        wl_buffer_add_listener (buffer, &buffer_listener, b);
        return b;
    }

    void blank ()
    {
        if (!m_surface) return;
        wl_surface_attach (m_surface, 0, 0, 0);
        wl_surface_commit (m_surface);
        if (m_display) wl_display_flush (m_display);
    }

    void update ()
    {
        if (!m_surface || !m_shm)
            return;

        if (!m_ui.is_visible ()) {
            blank ();
            return;
        }

        int w = 0, h = 0;
        m_ui.measure (w, h);

        ShmBuffer *b = create_buffer (w, h);
        if (!b) {
            blank ();
            return;
        }

        cairo_surface_t *cs = cairo_image_surface_create_for_data (
            static_cast<unsigned char *> (b->data), CAIRO_FORMAT_ARGB32,
            b->width, b->height, b->stride);
        cairo_t *cr = cairo_create (cs);
        m_ui.draw (cr);
        cairo_destroy (cr);
        cairo_surface_destroy (cs);

        // TODO(5c): output scale / HiDPI. Read the preferred buffer scale
        // (wl_surface enter -> wl_output, or fractional-scale-v1) and render
        // + wl_surface_set_buffer_scale accordingly; buffers are 1x for now.
        // TODO(5c): a per-frame throwaway shm buffer is simple but allocates
        // each update; reuse a buffer pool once the update rate matters.
        wl_surface_attach (m_surface, b->buffer, 0, 0);
        wl_surface_damage_buffer (m_surface, 0, 0, b->width, b->height);
        wl_surface_commit (m_surface);
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

    void handle_axis (uint32_t axis, wl_fixed_t value)
    {
        if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL)
            return;
        double v = wl_fixed_to_double (value);
        if (v < 0) {
            if (m_page_up_slot) m_page_up_slot ();
        } else if (v > 0) {
            if (m_page_down_slot) m_page_down_slot ();
        }
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
    if (surface == d->m_surface)
        d->m_pointer_on_surface = false;
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

void ptr_frame (void *, struct wl_pointer *) {}
void ptr_axis_source (void *, struct wl_pointer *, uint32_t) {}
void ptr_axis_stop (void *, struct wl_pointer *, uint32_t, uint32_t) {}
void ptr_axis_discrete (void *, struct wl_pointer *, uint32_t, int32_t) {}
void ptr_axis_value120 (void *, struct wl_pointer *, uint32_t, int32_t) {}
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

// input-popup-surface: the compositor tells us where the text cursor is
// within our surface. Not used yet (compositor default-positions the popup).
// TODO(5c): use this rectangle to align the candidate window to the cursor.
void popup_text_input_rectangle (void *, struct zwp_input_popup_surface_v2 *,
                                 int32_t, int32_t, int32_t, int32_t) {}

const struct zwp_input_popup_surface_v2_listener popup_listener = {
    popup_text_input_rectangle,
};

} // anonymous namespace

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

    if (seat) {
        d->m_pointer = wl_seat_get_pointer (seat);
        if (d->m_pointer)
            wl_pointer_add_listener (d->m_pointer, &pointer_listener, d);
    }

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
    return m_impl->m_popup != 0;
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
CandidatesWayland::signal_connect_candidate_selected (CandidateSlot slot)
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

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
