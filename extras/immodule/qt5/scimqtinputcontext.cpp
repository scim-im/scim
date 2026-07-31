/** @file scimqtinputcontext.cpp
 *  @brief native SCIM Qt5 input-method module (QPlatformInputContext).
 *
 *  This is the Qt counterpart of the GTK module: it embeds a SCIM
 *  CommonBackEnd, drives an IMEngineInstance directly, connects to the SCIM
 *  panel through PanelClient, and matches frontend hotkeys -- all over SCIM's
 *  own socket protocol, with no scim-bridge in between.
 *
 *  Unlike the GTK frontend (one GtkIMContext per text widget) Qt's platform
 *  input context is a single per-application object that follows the focused
 *  QObject, so this keeps a single input context.
 */

/*
 * Smart Common Input Method
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#define Uses_SCIM_DEBUG
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_CONFIG
#define Uses_SCIM_CONFIG_MODULE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_PANEL_CLIENT

#include <cstdlib>
#include <vector>
#include <algorithm>

// SCIM's signal library has methods named emit()/slot(); keep Qt's emit/signals
// /slots keyword macros out of the way (use Q_EMIT etc. instead, which we do not
// need here anyway).
#define QT_NO_KEYWORDS

#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>
#include <QtCore/QRectF>
#include <QtCore/QSocketNotifier>
#include <QtCore/QVariant>
#include <QtGui/QGuiApplication>
#include <QtGui/QInputMethod>
#include <QtGui/QInputMethodEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QPalette>
#include <QtGui/QTextCharFormat>

#include "scim_private.h"
#include "scim.h"

#ifdef SCIM_HAS_CANDIDATES
#include "scim_candidates.h"
#include <QtGui/QRasterWindow>
#include <QtGui/QPainter>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <cairo.h>
#endif

#include "scimqtinputcontext.h"

using namespace scim;

/* -------------------------------------------------------------------------- */
/* Per-context state (hidden behind the header's opaque Impl).                */
struct ScimQtInputContext::Impl
{
    IMEngineInstancePointer  si;
    int                      id;
    WideString               preedit_string;
    AttributeList            preedit_attrlist;
    int                      preedit_caret;
    bool                     is_on;
    bool                     use_preedit;
    bool                     preedit_started;
    bool                     shared_si;
    QPointer<QObject>        focus_object;
    int                      cursor_x;
    int                      cursor_y;
};

/* -------------------------------------------------------------------------- */
/* Shared backend state.                                                      */
static String                   _language;
static KeyboardLayout           _keyboard_layout   = SCIM_KEYBOARD_Default;
static int                      _valid_key_mask    = SCIM_KEY_AllMasks;

static FrontEndHotkeyMatcher    _frontend_hotkey_matcher;
static IMEngineHotkeyMatcher    _imengine_hotkey_matcher;

static IMEngineInstancePointer  _default_instance;

static ConfigModule            *_config_module     = 0;
static ConfigPointer            _config;
static BackEndPointer           _backend;

static IMEngineFactoryPointer   _fallback_factory;
static IMEngineInstancePointer  _fallback_instance;

static PanelClient              _panel_client;
static QSocketNotifier         *_panel_notifier    = 0;

static ScimQtInputContext      *_focused_ic        = 0;
static ScimQtInputContext      *_the_context       = 0;

static bool                     _scim_initialized  = false;
static int                      _instance_count    = 0;
static int                      _context_count     = 0;

static bool                     _on_the_spot         = true;
static bool                     _shared_input_method = false;

/* -------------------------------------------------------------------------- */
/* Forward declarations.                                                      */
static void     initialize                  (void);
static void     finalize                    (void);
static bool     check_socket_frontend       (void);
static bool     panel_initialize            (void);
static void     panel_finalize              (void);

static ScimQtInputContext *find_ic          (int id);

static void     attach_instance             (const IMEngineInstancePointer &si);
static void     set_ic_capabilities         (ScimQtInputContext *ic);
static bool     filter_hotkeys              (ScimQtInputContext *ic, const KeyEvent &key);
static void     turn_on_ic                  (ScimQtInputContext *ic);
static void     turn_off_ic                 (ScimQtInputContext *ic);
static void     do_focus_in                 (ScimQtInputContext *ic);
static void     do_focus_out                (ScimQtInputContext *ic);
static void     open_next_factory           (ScimQtInputContext *ic);
static void     open_previous_factory       (ScimQtInputContext *ic);
static void     open_specific_factory       (ScimQtInputContext *ic, const String &uuid);

static void     panel_req_update_screen        (ScimQtInputContext *ic);
static void     panel_req_show_help            (ScimQtInputContext *ic);
static void     panel_req_show_factory_menu    (ScimQtInputContext *ic);
static void     panel_req_update_factory_info  (ScimQtInputContext *ic);
static void     panel_req_focus_in             (ScimQtInputContext *ic);

#ifdef SCIM_HAS_CANDIDATES
// Defined further down; needed by the focus/turn-on reset above it.
static void     candidates_hide            ();
// Renderer-side preedit/aux, used when the client cannot draw preedit inline
// (and always for aux, which has no client-side path). Defined further down,
// where ScimCandidatesWindow is a complete type.
static void candidates_preedit_show   (ScimQtInputContext *ic);
static void candidates_preedit_hide   ();
static void candidates_preedit_update (const WideString &str, const AttributeList &attrs);
static void candidates_preedit_caret  (int caret);
static void candidates_aux_show       (ScimQtInputContext *ic);
static void candidates_aux_hide       ();
static void candidates_aux_update     (const WideString &str, const AttributeList &attrs);
#endif

static KeyEvent keyevent_qt_to_scim         (const QKeyEvent *qe);
static void     update_preedit_in_client    (ScimQtInputContext *ic);
static void     hide_preedit_in_client      (ScimQtInputContext *ic);
static void     reload_config_callback      (const ConfigPointer &config);
static void     fallback_commit_string_cb   (IMEngineInstanceBase *si, const WideString &str);

/* -------------------------------------------------------------------------- */
/* Small helpers.                                                             */
static inline QString wstr_to_qstr (const WideString &wstr)
{
    return QString::fromUtf8 (utf8_wcstombs (wstr).c_str ());
}

static ScimQtInputContext *find_ic (int id)
{
    if (_the_context && _the_context->impl && _the_context->impl->id == id)
        return _the_context;
    return 0;
}

/* -------------------------------------------------------------------------- */
/* Panel slot functions (callbacks from the SCIM panel).                      */
static void panel_slot_reload_config (int /* context */)
{
    _config->reload ();
}

static void panel_slot_exit (int /* context */)
{
    finalize ();
}

static void panel_slot_lookup_table_page_up (int context)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl && !ic->impl->si.null ()) {
        _panel_client.prepare (ic->impl->id);
        ic->impl->si->lookup_table_page_up ();
        _panel_client.send ();
    }
}

static void panel_slot_lookup_table_page_down (int context)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl && !ic->impl->si.null ()) {
        _panel_client.prepare (ic->impl->id);
        ic->impl->si->lookup_table_page_down ();
        _panel_client.send ();
    }
}

static void panel_slot_trigger_property (int context, const String &property)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl && !ic->impl->si.null ()) {
        _panel_client.prepare (ic->impl->id);
        ic->impl->si->trigger_property (property);
        _panel_client.send ();
    }
}

static void panel_slot_process_helper_event (int context, const String &target_uuid,
                                             const String &helper_uuid, const Transaction &trans)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl && !ic->impl->si.null () &&
        ic->impl->si->get_factory_uuid () == target_uuid) {
        _panel_client.prepare (ic->impl->id);
        ic->impl->si->process_helper_event (helper_uuid, trans);
        _panel_client.send ();
    }
}

static void panel_slot_move_preedit_caret (int context, int caret_pos)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl && !ic->impl->si.null ()) {
        _panel_client.prepare (ic->impl->id);
        ic->impl->si->move_preedit_caret (caret_pos);
        _panel_client.send ();
    }
}

static void panel_slot_select_candidate (int context, int cand_index)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl && !ic->impl->si.null ()) {
        _panel_client.prepare (ic->impl->id);
        ic->impl->si->select_candidate (cand_index);
        _panel_client.send ();
    }
}

static void panel_slot_process_key_event (int context, const KeyEvent &key)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl) {
        _panel_client.prepare (ic->impl->id);
        if (!filter_hotkeys (ic, key)) {
            if (!_focused_ic || !_focused_ic->impl->is_on ||
                _focused_ic->impl->si.null () ||
                !_focused_ic->impl->si->process_key_event (key)) {
                _fallback_instance->process_key_event (key);
            }
        }
        _panel_client.send ();
    }
}

static void panel_slot_commit_string (int context, const WideString &wstr)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl && ic->impl->focus_object) {
        QInputMethodEvent ev;
        ev.setCommitString (wstr_to_qstr (wstr));
        ic->sendEvent (ev);
    }
}

static void panel_slot_forward_key_event (int context, const KeyEvent &key)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl) {
        // Let the fallback engine turn the key into a commit where it can.
        // TODO: inject a real QKeyEvent for true forwarding.
        _fallback_instance->process_key_event (key);
    }
}

static void panel_slot_request_help (int context)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl) {
        _panel_client.prepare (ic->impl->id);
        panel_req_show_help (ic);
        _panel_client.send ();
    }
}

static void panel_slot_request_factory_menu (int context)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl) {
        _panel_client.prepare (ic->impl->id);
        panel_req_show_factory_menu (ic);
        _panel_client.send ();
    }
}

static void panel_slot_change_factory (int context, const String &uuid)
{
    ScimQtInputContext *ic = find_ic (context);
    if (ic && ic->impl) {
        _panel_client.prepare (ic->impl->id);
        open_specific_factory (ic, uuid);
        _panel_client.send ();
    }
}

/* -------------------------------------------------------------------------- */
/* Panel request functions.                                                   */
static void panel_req_update_screen (ScimQtInputContext *ic)
{
    // Report screen 0; multi-screen coordinate reporting is a Wayland-era
    // concern handled together with candidate anchoring later.
    if (ic && ic->impl)
        _panel_client.update_screen (ic->impl->id, 0);
}

static void panel_req_show_help (ScimQtInputContext *ic)
{
    String help = String (_("Smart Common Input Method platform ")) +
                  String (SCIM_VERSION) +
                  String (_("\n(C) 2002-2005 James Su <suzhe@tsinghua.org.cn>\n\n"));

    if (ic && ic->impl && !ic->impl->si.null ()) {
        IMEngineFactoryPointer sf = _backend->get_factory (ic->impl->si->get_factory_uuid ());
        if (!sf.null ()) {
            help += utf8_wcstombs (sf->get_name ())    + String (_(":\n\n"));
            help += utf8_wcstombs (sf->get_authors ())  + String (_("\n\n"));
            help += utf8_wcstombs (sf->get_help ())     + String (_("\n\n"));
            help += utf8_wcstombs (sf->get_credits ());
        }
        _panel_client.show_help (ic->impl->id, help);
    }
}

static void panel_req_show_factory_menu (ScimQtInputContext *ic)
{
    std::vector<IMEngineFactoryPointer> factories;
    std::vector<PanelFactoryInfo>       menu;

    _backend->get_factories_for_encoding (factories, "UTF-8");

    for (size_t i = 0; i < factories.size (); ++i)
        menu.push_back (PanelFactoryInfo (factories [i]->get_uuid (),
                                          utf8_wcstombs (factories [i]->get_name ()),
                                          factories [i]->get_language (),
                                          factories [i]->get_icon_file (),
                                    factories [i]->get_symbol ()));

    if (menu.size ())
        _panel_client.show_factory_menu (ic->impl->id, menu);
}

static void panel_req_update_factory_info (ScimQtInputContext *ic)
{
    if (ic && ic->impl && ic == _focused_ic) {
        PanelFactoryInfo info;
        if (ic->impl->is_on && !ic->impl->si.null ()) {
            IMEngineFactoryPointer sf = _backend->get_factory (ic->impl->si->get_factory_uuid ());
            info = PanelFactoryInfo (sf->get_uuid (), utf8_wcstombs (sf->get_name ()),
                                     sf->get_language (), sf->get_icon_file (), sf->get_symbol ());
        } else {
            info = PanelFactoryInfo (String (""), String (_("English/Keyboard")),
                                     String ("C"), String (SCIM_KEYBOARD_ICON_FILE),
                                     String (_("En")));
        }
        _panel_client.update_factory_info (ic->impl->id, info);
    }
}

static void panel_req_focus_in (ScimQtInputContext *ic)
{
    if (ic && ic->impl && !ic->impl->si.null ())
        _panel_client.focus_in (ic->impl->id, ic->impl->si->get_factory_uuid ());
}


/* -------------------------------------------------------------------------- */
/* Hotkeys and IC on/off.                                                     */
static bool filter_hotkeys (ScimQtInputContext *ic, const KeyEvent &key)
{
    bool ret = false;

    _frontend_hotkey_matcher.push_key_event (key);
    _imengine_hotkey_matcher.push_key_event (key);

    FrontEndHotkeyAction hotkey_action = _frontend_hotkey_matcher.get_match_result ();

    if (hotkey_action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        if (!ic->impl->is_on) turn_on_ic (ic); else turn_off_ic (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_ON) {
        if (!ic->impl->is_on) turn_on_ic (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_OFF) {
        if (ic->impl->is_on) turn_off_ic (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        open_next_factory (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        open_previous_factory (ic);
        ret = true;
    } else if (hotkey_action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        panel_req_show_factory_menu (ic);
        ret = true;
    } else if (_imengine_hotkey_matcher.is_matched ()) {
        String sfid = _imengine_hotkey_matcher.get_match_result ();
        open_specific_factory (ic, sfid);
        ret = true;
    }

    return ret;
}

static void turn_on_ic (ScimQtInputContext *ic)
{
    if (ic && ic->impl && !ic->impl->si.null () && !ic->impl->is_on) {
        ic->impl->is_on = true;

        if (ic == _focused_ic) {
            panel_req_focus_in (ic);
            panel_req_update_screen (ic);
            panel_req_update_factory_info (ic);
            _panel_client.turn_on (ic->impl->id);
#ifdef SCIM_HAS_CANDIDATES
            // This context just became active. The in-process renderer is shared by
            // every context in this process, so clear anything the previously focused
            // one left on screen before we start drawing.
            candidates_hide ();
#endif
            ic->impl->si->focus_in ();
        }

        if (_shared_input_method)
            _config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), true);
    }
}

static void turn_off_ic (ScimQtInputContext *ic)
{
    if (ic && ic->impl && !ic->impl->si.null () && ic->impl->is_on) {
        ic->impl->is_on = false;

        if (ic == _focused_ic) {
            ic->impl->si->focus_out ();
            panel_req_update_factory_info (ic);
            _panel_client.turn_off (ic->impl->id);
        }

        if (_shared_input_method)
            _config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);

        hide_preedit_in_client (ic);
    }
}

static void set_ic_capabilities (ScimQtInputContext *ic)
{
    if (ic && ic->impl && !ic->impl->si.null ()) {
        unsigned int cap = SCIM_CLIENT_CAP_ALL_CAPABILITIES;
        if (!_on_the_spot || !ic->impl->use_preedit)
            cap -= SCIM_CLIENT_CAP_ONTHESPOT_PREEDIT;
        ic->impl->si->update_client_capabilities (cap);
    }
}

static void do_focus_in (ScimQtInputContext *ic)
{
    if (_focused_ic && _focused_ic != ic)
        do_focus_out (_focused_ic);

    if (!ic || !ic->impl) return;

    _focused_ic = ic;
    _panel_client.prepare (ic->impl->id);

    panel_req_focus_in (ic);
    panel_req_update_screen (ic);
    panel_req_update_factory_info (ic);

    if (ic->impl->is_on && !ic->impl->si.null ()) {
        _panel_client.turn_on (ic->impl->id);
#ifdef SCIM_HAS_CANDIDATES
        // This context just became active. The in-process renderer is shared by
        // every context in this process, so clear anything the previously focused
        // one left on screen before we start drawing.
        candidates_hide ();
#endif
        ic->impl->si->focus_in ();
    } else {
        _panel_client.turn_off (ic->impl->id);
    }

    _panel_client.send ();
}

static void do_focus_out (ScimQtInputContext *ic)
{
    if (!ic || !ic->impl) return;

    if (_focused_ic == ic) {
        _panel_client.prepare (ic->impl->id);
        if (!ic->impl->si.null ()) ic->impl->si->focus_out ();
        _panel_client.turn_off (ic->impl->id);
        _panel_client.focus_out (ic->impl->id);
        _panel_client.send ();
        _focused_ic = 0;
    }
}

static void open_specific_factory (ScimQtInputContext *ic, const String &uuid)
{
    if (!ic || !ic->impl) return;

    if (!ic->impl->si.null () && ic->impl->si->get_factory_uuid () == uuid) {
        turn_on_ic (ic);
        return;
    }

    IMEngineFactoryPointer sf = _backend->get_factory (uuid);

    if (uuid.length () && !sf.null ()) {
        turn_off_ic (ic);
        int old_id = ic->impl->si.null () ? _instance_count++ : ic->impl->si->get_id ();
        ic->impl->si = sf->create_instance ("UTF-8", old_id);
        ic->impl->si->set_frontend_data (static_cast<void *> (ic));
        ic->impl->preedit_string = WideString ();
        ic->impl->preedit_caret = 0;
        attach_instance (ic->impl->si);
        _backend->set_default_factory (_language, sf->get_uuid ());
        _panel_client.register_input_context (ic->impl->id, sf->get_uuid ());
        set_ic_capabilities (ic);
        turn_on_ic (ic);

        if (_shared_input_method) {
            _default_instance = ic->impl->si;
            ic->impl->shared_si = true;
        }
    } else {
        turn_off_ic (ic);
    }
}

static void open_next_factory (ScimQtInputContext *ic)
{
    if (!ic || !ic->impl || ic->impl->si.null ()) return;
    IMEngineFactoryPointer sf =
        _backend->get_next_factory ("", "UTF-8", ic->impl->si->get_factory_uuid ());
    if (!sf.null ())
        open_specific_factory (ic, sf->get_uuid ());
}

static void open_previous_factory (ScimQtInputContext *ic)
{
    if (!ic || !ic->impl || ic->impl->si.null ()) return;
    IMEngineFactoryPointer sf =
        _backend->get_previous_factory ("", "UTF-8", ic->impl->si->get_factory_uuid ());
    if (!sf.null ())
        open_specific_factory (ic, sf->get_uuid ());
}

/* -------------------------------------------------------------------------- */
/* Preedit rendering into the focused client.                                 */
static void build_preedit_attributes (const WideString &str, const AttributeList &attrs,
                                      QList<QInputMethodEvent::Attribute> &out)
{
    // Underline the whole preedit by default.
    QTextCharFormat base;
    base.setFontUnderline (true);
    out.append (QInputMethodEvent::Attribute (QInputMethodEvent::TextFormat,
                                              0, (int) str.length (), base));

    // Highlight reversed/highlighted segments reported by the engine.
    for (size_t i = 0; i < attrs.size (); ++i) {
        const Attribute &a = attrs [i];
        if (a.get_type () == SCIM_ATTR_DECORATE &&
            (a.get_value () == SCIM_ATTR_DECORATE_REVERSE ||
             a.get_value () == SCIM_ATTR_DECORATE_HIGHLIGHT)) {
            QTextCharFormat hi;
            hi.setFontUnderline (true);
            hi.setBackground (QGuiApplication::palette ().highlight ());
            hi.setForeground (QGuiApplication::palette ().highlightedText ());
            out.append (QInputMethodEvent::Attribute (QInputMethodEvent::TextFormat,
                                                      (int) a.get_start (),
                                                      (int) a.get_length (), hi));
        }
    }
}

static void update_preedit_in_client (ScimQtInputContext *ic)
{
    if (!ic || !ic->impl || !ic->impl->focus_object) return;

    QString pre = wstr_to_qstr (ic->impl->preedit_string);

    QList<QInputMethodEvent::Attribute> attrs;
    build_preedit_attributes (ic->impl->preedit_string, ic->impl->preedit_attrlist, attrs);
    attrs.append (QInputMethodEvent::Attribute (QInputMethodEvent::Cursor,
                                                ic->impl->preedit_caret, 1, QVariant ()));

    QInputMethodEvent ev (pre, attrs);
    ic->sendEvent (ev);
}

static void hide_preedit_in_client (ScimQtInputContext *ic)
{
    if (!ic || !ic->impl || !ic->impl->focus_object) return;
    QList<QInputMethodEvent::Attribute> attrs;
    QInputMethodEvent ev (QString (), attrs);
    ic->sendEvent (ev);
    ic->impl->preedit_started = false;
}

/* -------------------------------------------------------------------------- */
/* IMEngineInstance slot functions.                                           */
static void slot_show_preedit_string (IMEngineInstanceBase *si)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic) {
        if (ic->impl->use_preedit) {
            ic->impl->preedit_started = true;
            update_preedit_in_client (ic);
        }
#ifdef SCIM_HAS_CANDIDATES
        else {
            // The client cannot draw preedit inline, so the in-process
            // renderer does it (matching the x11 frontend).
            candidates_preedit_show (ic);
        }
#endif
    }
}

static void slot_show_aux_string (IMEngineInstanceBase *si)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic) {
#ifdef SCIM_HAS_CANDIDATES
        // There is no client-side path for the aux string; the renderer is the
        // only place it can appear.
        candidates_aux_show (ic);
#endif
    }
}
#ifdef SCIM_HAS_CANDIDATES
static void candidates_route_click (CandidatesUI::HitType hit, int idx);

// Cairo candidate renderer hosted in a QtGui-only raster window (no QtWidgets
// dependency). Drawn by blitting the shared CandidatesUI's Cairo output.
class ScimCandidatesWindow : public QRasterWindow
{
public:
    CandidatesUI ui;
    ScimCandidatesWindow () { setFlags (Qt::ToolTip | Qt::FramelessWindowHint); }

    void refresh () {
        int w = 0, h = 0;
        ui.measure (w, h);
        if (w > 0 && h > 0) resize (w, h);
        requestUpdate ();
    }

protected:
    void paintEvent (QPaintEvent *) override {
        int w = (int) width (), h = (int) height ();
        if (w <= 0 || h <= 0) return;
        cairo_surface_t *surf = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
        cairo_t *cr = cairo_create (surf);
        ui.draw (cr);
        cairo_surface_flush (surf);
        QImage img (cairo_image_surface_get_data (surf), w, h,
                    cairo_image_surface_get_stride (surf),
                    QImage::Format_ARGB32_Premultiplied);
        QPainter p (this);
        p.drawImage (0, 0, img);
        cairo_destroy (cr);
        cairo_surface_destroy (surf);
    }

    void mousePressEvent (QMouseEvent *ev) override {
        int idx = -1;
        CandidatesUI::HitType hit = ui.hit_test (ev->x (), ev->y (), idx);
        candidates_route_click (hit, idx);
    }
};

static ScimCandidatesWindow *_candidates_window = 0;

static void candidates_route_click (CandidatesUI::HitType hit, int idx)
{
    if (!_focused_ic || !_focused_ic->impl || _focused_ic->impl->si.null ())
        return;
    IMEngineInstancePointer si = _focused_ic->impl->si;
    _panel_client.prepare (_focused_ic->impl->id);
    if (hit == CandidatesUI::HIT_CANDIDATE && idx >= 0)
        si->select_candidate (idx);
    else if (hit == CandidatesUI::HIT_PREV_PAGE)
        si->lookup_table_page_up ();
    else if (hit == CandidatesUI::HIT_NEXT_PAGE)
        si->lookup_table_page_down ();
    _panel_client.send ();
}

static void candidates_ensure ()
{
    if (!_candidates_window) {
        _candidates_window = new ScimCandidatesWindow ();
        if (!_config.null ())
            _candidates_window->ui.set_theme (scim_candidates_theme_from_config (_config));
    }
}

static void candidates_show (ScimQtInputContext *ic)
{
    candidates_ensure ();
    // cursor_x/y are window-local. On X11 map to global via the focus window's
    // origin. On Wayland a client cannot set absolute positions: the window is
    // an xdg_popup anchored to the transient parent, so keep the parent-relative
    // (window-local) offset and let the compositor place it.
    QWindow *fw = QGuiApplication::focusWindow ();
    QPoint pos (ic->impl->cursor_x, ic->impl->cursor_y);
    if (fw) {
        _candidates_window->setTransientParent (fw);
        if (!QGuiApplication::platformName ().startsWith (QLatin1String ("wayland")))
            pos = fw->position () + pos;
    }
    _candidates_window->refresh ();
    _candidates_window->setPosition (pos);
    _candidates_window->show ();
}

static void candidates_hide ()
{
    if (_candidates_window) _candidates_window->hide ();
}

static void candidates_update (const LookupTable &table)
{
    candidates_ensure ();
    _candidates_window->ui.update_lookup_table (table);
    _candidates_window->refresh ();
}

static void candidates_preedit_show (ScimQtInputContext *ic)
{
    candidates_ensure ();
    _candidates_window->ui.show_preedit_string ();
    _candidates_window->refresh ();
    candidates_show (ic);
}

static void candidates_preedit_hide ()
{
    candidates_ensure ();
    _candidates_window->ui.hide_preedit_string ();
    _candidates_window->refresh ();
}

static void candidates_preedit_update (const WideString &str, const AttributeList &attrs)
{
    candidates_ensure ();
    _candidates_window->ui.update_preedit_string (str, attrs);
    _candidates_window->refresh ();
}

static void candidates_preedit_caret (int caret)
{
    candidates_ensure ();
    _candidates_window->ui.update_preedit_caret (caret);
    _candidates_window->refresh ();
}

static void candidates_aux_show (ScimQtInputContext *ic)
{
    candidates_ensure ();
    _candidates_window->ui.show_aux_string ();
    _candidates_window->refresh ();
    candidates_show (ic);
}

static void candidates_aux_hide ()
{
    candidates_ensure ();
    _candidates_window->ui.hide_aux_string ();
    _candidates_window->refresh ();
}

static void candidates_aux_update (const WideString &str, const AttributeList &attrs)
{
    candidates_ensure ();
    _candidates_window->ui.update_aux_string (str, attrs);
    _candidates_window->refresh ();
}

static void candidates_finalize ()
{
    if (_candidates_window) {
        delete _candidates_window;
        _candidates_window = 0;
    }
}
#endif // SCIM_HAS_CANDIDATES

static void slot_show_lookup_table (IMEngineInstanceBase *si)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic) {
#ifdef SCIM_HAS_CANDIDATES
        candidates_show (ic);
#endif
    }
}

static void slot_hide_preedit_string (IMEngineInstanceBase *si)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic) {
        ic->impl->preedit_string = WideString ();
        ic->impl->preedit_caret  = 0;
        if (ic->impl->use_preedit)
            hide_preedit_in_client (ic);
#ifdef SCIM_HAS_CANDIDATES
        else {
            candidates_preedit_hide ();
        }
#endif
    }
}

static void slot_hide_aux_string (IMEngineInstanceBase *si)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic) {
#ifdef SCIM_HAS_CANDIDATES
        candidates_aux_hide ();
#endif
    }
}
static void slot_hide_lookup_table (IMEngineInstanceBase *si)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic) {
#ifdef SCIM_HAS_CANDIDATES
        candidates_hide ();
#endif
    }
}

static void slot_update_preedit_caret (IMEngineInstanceBase *si, int caret)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic && ic->impl->preedit_caret != caret) {
        ic->impl->preedit_caret = caret;
        if (ic->impl->use_preedit)
            update_preedit_in_client (ic);
#ifdef SCIM_HAS_CANDIDATES
        else {
            candidates_preedit_caret (caret);
        }
#endif
    }
}

static void slot_update_preedit_string (IMEngineInstanceBase *si,
                                        const WideString &str, const AttributeList &attrs)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic) {
        ic->impl->preedit_string   = str;
        ic->impl->preedit_attrlist = attrs;
        if (ic->impl->use_preedit) {
            ic->impl->preedit_started = true;
            ic->impl->preedit_caret   = (int) str.length ();
            update_preedit_in_client (ic);
        }
#ifdef SCIM_HAS_CANDIDATES
        else
            candidates_preedit_update (str, attrs);
#endif
    }
}

static void slot_update_aux_string (IMEngineInstanceBase *si,
                                    const WideString &str, const AttributeList &attrs)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());

    if (ic && ic->impl && _focused_ic == ic) {
#ifdef SCIM_HAS_CANDIDATES
        candidates_aux_update (str, attrs);
#endif
    }
}
static void slot_commit_string (IMEngineInstanceBase *si, const WideString &str)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && ic->impl->focus_object) {
        QInputMethodEvent ev;
        ev.setCommitString (wstr_to_qstr (str));
        ic->sendEvent (ev);
    }
}

static void slot_forward_key_event (IMEngineInstanceBase *si, const KeyEvent &key)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && _focused_ic == ic) {
        // Let the fallback engine commit the key as text where it can.
        // TODO: inject a real QKeyEvent to the focus object for full forwarding.
        _fallback_instance->process_key_event (key);
    }
}

static void slot_update_lookup_table (IMEngineInstanceBase *si, const LookupTable &table)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic) {
#ifdef SCIM_HAS_CANDIDATES
        candidates_update (table);
#endif
    }
}

static void slot_register_properties (IMEngineInstanceBase *si, const PropertyList &properties)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.register_properties (ic->impl->id, properties);
}

static void slot_update_property (IMEngineInstanceBase *si, const Property &property)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic)
        _panel_client.update_property (ic->impl->id, property);
}

static void slot_beep (IMEngineInstanceBase *si)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl && _focused_ic == ic) {
        // TODO: audible bell (QApplication::beep needs QtWidgets).
    }
}

static void slot_start_helper (IMEngineInstanceBase *si, const String &helper_uuid)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl)
        _panel_client.start_helper (ic->impl->id, helper_uuid);
}

static void slot_stop_helper (IMEngineInstanceBase *si, const String &helper_uuid)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl)
        _panel_client.stop_helper (ic->impl->id, helper_uuid);
}

static void slot_send_helper_event (IMEngineInstanceBase *si, const String &helper_uuid,
                                    const Transaction &trans)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (ic && ic->impl)
        _panel_client.send_helper_event (ic->impl->id, helper_uuid, trans);
}

static bool slot_get_surrounding_text (IMEngineInstanceBase *si, WideString &text, int &cursor,
                                       int maxlen_before, int maxlen_after)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (!ic || !ic->impl || _focused_ic != ic || !ic->impl->focus_object)
        return false;

    QInputMethodQueryEvent query (Qt::ImSurroundingText | Qt::ImCursorPosition);
    QCoreApplication::sendEvent (ic->impl->focus_object, &query);

    QString surrounding = query.value (Qt::ImSurroundingText).toString ();
    int     cursor_index = query.value (Qt::ImCursorPosition).toInt ();

    if (cursor_index < 0 || cursor_index > surrounding.length ())
        return false;

    WideString before = utf8_mbstowcs (surrounding.left (cursor_index).toUtf8 ().constData ());
    WideString after  = utf8_mbstowcs (surrounding.mid (cursor_index).toUtf8 ().constData ());

    if (maxlen_before > 0 && (size_t) maxlen_before < before.length ())
        before = WideString (before.end () - maxlen_before, before.end ());
    else if (maxlen_before == 0)
        before = WideString ();

    if (maxlen_after > 0 && (size_t) maxlen_after < after.length ())
        after = WideString (after.begin (), after.begin () + maxlen_after);
    else if (maxlen_after == 0)
        after = WideString ();

    text   = before + after;
    cursor = (int) before.length ();
    return true;
}

static bool slot_delete_surrounding_text (IMEngineInstanceBase *si, int offset, int len)
{
    ScimQtInputContext *ic = static_cast<ScimQtInputContext *> (si->get_frontend_data ());
    if (!ic || !ic->impl || _focused_ic != ic || !ic->impl->focus_object)
        return false;

    QInputMethodEvent ev;
    ev.setCommitString (QString (), offset, len);
    ic->sendEvent (ev);
    return true;
}

/* -------------------------------------------------------------------------- */
static void attach_instance (const IMEngineInstancePointer &si)
{
    si->signal_connect_show_preedit_string   (slot (slot_show_preedit_string));
    si->signal_connect_show_aux_string       (slot (slot_show_aux_string));
    si->signal_connect_show_lookup_table     (slot (slot_show_lookup_table));
    si->signal_connect_hide_preedit_string   (slot (slot_hide_preedit_string));
    si->signal_connect_hide_aux_string       (slot (slot_hide_aux_string));
    si->signal_connect_hide_lookup_table     (slot (slot_hide_lookup_table));
    si->signal_connect_update_preedit_caret  (slot (slot_update_preedit_caret));
    si->signal_connect_update_preedit_string (slot (slot_update_preedit_string));
    si->signal_connect_update_aux_string     (slot (slot_update_aux_string));
    si->signal_connect_update_lookup_table   (slot (slot_update_lookup_table));
    si->signal_connect_commit_string         (slot (slot_commit_string));
    si->signal_connect_forward_key_event     (slot (slot_forward_key_event));
    si->signal_connect_register_properties   (slot (slot_register_properties));
    si->signal_connect_update_property       (slot (slot_update_property));
    si->signal_connect_beep                  (slot (slot_beep));
    si->signal_connect_start_helper          (slot (slot_start_helper));
    si->signal_connect_stop_helper           (slot (slot_stop_helper));
    si->signal_connect_send_helper_event     (slot (slot_send_helper_event));
    si->signal_connect_get_surrounding_text  (slot (slot_get_surrounding_text));
    si->signal_connect_delete_surrounding_text (slot (slot_delete_surrounding_text));
}

/* -------------------------------------------------------------------------- */
/* Key event conversion.                                                      */
static KeyEvent keyevent_qt_to_scim (const QKeyEvent *qe)
{
    KeyEvent key;

    // On the xcb/X11 (and xkb-based Wayland) platforms nativeVirtualKey() is
    // the X keysym, which is exactly what SCIM's KeyEvent.code expects.
    key.code = qe->nativeVirtualKey ();

    Qt::KeyboardModifiers mods = qe->modifiers ();
    if (mods & Qt::ShiftModifier)   key.mask |= SCIM_KEY_ShiftMask;
    if (mods & Qt::ControlModifier) key.mask |= SCIM_KEY_ControlMask;
    if (mods & Qt::AltModifier)     key.mask |= SCIM_KEY_AltMask;
    if (mods & Qt::MetaModifier)    key.mask |= SCIM_KEY_MetaMask;
    if (mods & Qt::KeypadModifier)  key.mask |= SCIM_KEY_NumLockMask;

    if (qe->type () == QEvent::KeyRelease)
        key.mask |= SCIM_KEY_ReleaseMask;

    key.mask  &= _valid_key_mask;
    key.layout = _keyboard_layout;
    return key;
}

/* -------------------------------------------------------------------------- */
/* Config reload / fallback commit.                                           */
static void reload_config_callback (const ConfigPointer &config)
{
    _frontend_hotkey_matcher.load_hotkeys (config);
    _imengine_hotkey_matcher.load_hotkeys (config);

    KeyEvent key;
    scim_string_to_key (key,
        config->read (String (SCIM_CONFIG_HOTKEYS_FRONTEND_VALID_KEY_MASK),
                      String ("Shift+Control+Alt+Lock")));

    _valid_key_mask  = (key.mask > 0) ? key.mask : 0xFFFF;
    _valid_key_mask |= SCIM_KEY_ReleaseMask;
    _valid_key_mask |= SCIM_KEY_QuirkKanaRoMask;

    _on_the_spot         = config->read (String (SCIM_CONFIG_FRONTEND_ON_THE_SPOT), _on_the_spot);
    _shared_input_method = config->read (String (SCIM_CONFIG_FRONTEND_SHARED_INPUT_METHOD), _shared_input_method);

    scim_global_config_flush ();
    _keyboard_layout = scim_get_default_keyboard_layout ();

    // Re-apply the candidate appearance so a font/color change takes effect
    // without restarting.
    if (_candidates_window)
        _candidates_window->ui.set_theme (scim_candidates_theme_from_config (config));
}

static void fallback_commit_string_cb (IMEngineInstanceBase * /* si */, const WideString &str)
{
    if (_focused_ic && _focused_ic->impl && _focused_ic->impl->focus_object) {
        QInputMethodEvent ev;
        ev.setCommitString (wstr_to_qstr (str));
        _focused_ic->sendEvent (ev);
    }
}

/* -------------------------------------------------------------------------- */
/* Panel connection (integrated into the Qt event loop via QSocketNotifier).  */
static bool panel_initialize (void)
{
    String display_name;
    const char *p = getenv ("DISPLAY");
    if (p) display_name = String (p);

    if (_panel_client.open_connection (_config->get_name ()) >= 0) {
        int fd = _panel_client.get_connection_number ();

        _panel_notifier = new QSocketNotifier (fd, QSocketNotifier::Read);
        QObject::connect (_panel_notifier, &QSocketNotifier::activated, [] () {
            if (!_panel_client.filter_event ()) {
                panel_finalize ();
                panel_initialize ();
            }
        });
        return true;
    }
    return false;
}

static void panel_finalize (void)
{
    _panel_client.close_connection ();

    if (_panel_notifier) {
        _panel_notifier->setEnabled (false);
        _panel_notifier->deleteLater ();
        _panel_notifier = 0;
    }
}

/* -------------------------------------------------------------------------- */
static bool check_socket_frontend (void)
{
    SocketAddress address;
    SocketClient  client;
    uint32        magic;

    address.set_address (scim_get_default_socket_frontend_address ());

    if (!client.connect (address))
        return false;

    return scim_socket_open_connection (magic, String ("ConnectionTester"),
                                        String ("SocketFrontEnd"), client, 1000);
}

static void initialize (void)
{
    std::vector<String> config_list;
    std::vector<String> engine_list;
    std::vector<String> load_engine_list;

    bool   manual = false;
    bool   socket = true;
    String config_module_name;

    _language = scim_get_locale_language (scim_get_current_locale ());

    scim_get_imengine_module_list (engine_list);
    scim_get_config_module_list (config_list);

    if (std::find (engine_list.begin (), engine_list.end (), "socket") == engine_list.end () ||
        std::find (config_list.begin (), config_list.end (), "socket") == config_list.end ())
        socket = false;

    if (config_list.size ()) {
        config_module_name = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_CONFIG_MODULE, String ("simple"));
        if (std::find (config_list.begin (), config_list.end (), config_module_name) == config_list.end ())
            config_module_name = config_list [0];
    } else {
        config_module_name = "dummy";
    }

    const char *engine_list_str = getenv ("QT_IM_SCIM_IMENGINE_MODULES");
    if (engine_list_str != NULL) {
        std::vector<String> spec_engine_list;
        scim_split_string_list (spec_engine_list, engine_list_str, ',');
        load_engine_list.clear ();
        for (size_t i = 0; i < spec_engine_list.size (); ++i)
            if (std::find (engine_list.begin (), engine_list.end (), spec_engine_list [i]) != engine_list.end ())
                load_engine_list.push_back (spec_engine_list [i]);
        manual = true;
    }

    if (config_module_name == "socket" ||
        std::find (load_engine_list.begin (), load_engine_list.end (), "socket") != load_engine_list.end ())
        socket = false;

    if (scim_get_default_socket_frontend_address () != scim_get_default_socket_imengine_address () &&
        scim_get_default_socket_frontend_address () != scim_get_default_socket_config_address ())
        socket = false;

    if (socket) {
        if (!check_socket_frontend ()) {
            char *new_argv [] = { const_cast<char *> ("--no-stay"), 0 };
            scim_launch (true,
                         config_module_name,
                         (load_engine_list.size () ? scim_combine_string_list (load_engine_list, ',') : "all"),
                         "socket",
                         new_argv);
            manual = false;
        }

        if (!manual) {
            for (int i = 0; i < 100; ++i) {
                if (check_socket_frontend ()) {
                    config_module_name = "socket";
                    load_engine_list.clear ();
                    load_engine_list.push_back ("socket");
                    break;
                }
                scim_usleep (100000);
            }
        }
    }

    if (config_module_name != "dummy") {
        _config_module = new ConfigModule (config_module_name);
        if (_config_module != NULL && _config_module->valid ())
            _config = _config_module->create_config ();
    }

    if (_config.null ()) {
        if (_config_module) delete _config_module;
        _config_module = NULL;
        _config = new DummyConfig ();
        config_module_name = "dummy";
    }

    reload_config_callback (_config);
    _config->signal_connect_reload (slot (reload_config_callback));

    _backend = new CommonBackEnd (_config, load_engine_list.size () ? load_engine_list : engine_list);
    if (_backend.null ()) {
        std::cerr << "SCIM Qt IM Module: Cannot create BackEnd Object!\n";
    } else {
        _fallback_factory = _backend->get_factory (SCIM_COMPOSE_KEY_FACTORY_UUID);
    }

    if (_fallback_factory.null ()) _fallback_factory = new DummyIMEngineFactory ();

    _fallback_instance = _fallback_factory->create_instance (String ("UTF-8"), 0);
    _fallback_instance->signal_connect_commit_string (slot (fallback_commit_string_cb));

    _panel_client.signal_connect_reload_config                 (slot (panel_slot_reload_config));
    _panel_client.signal_connect_exit                          (slot (panel_slot_exit));
    _panel_client.signal_connect_lookup_table_page_up          (slot (panel_slot_lookup_table_page_up));
    _panel_client.signal_connect_lookup_table_page_down        (slot (panel_slot_lookup_table_page_down));
    _panel_client.signal_connect_trigger_property              (slot (panel_slot_trigger_property));
    _panel_client.signal_connect_process_helper_event          (slot (panel_slot_process_helper_event));
    _panel_client.signal_connect_move_preedit_caret            (slot (panel_slot_move_preedit_caret));
    _panel_client.signal_connect_select_candidate              (slot (panel_slot_select_candidate));
    _panel_client.signal_connect_process_key_event             (slot (panel_slot_process_key_event));
    _panel_client.signal_connect_commit_string                 (slot (panel_slot_commit_string));
    _panel_client.signal_connect_forward_key_event             (slot (panel_slot_forward_key_event));
    _panel_client.signal_connect_request_help                  (slot (panel_slot_request_help));
    _panel_client.signal_connect_request_factory_menu          (slot (panel_slot_request_factory_menu));
    _panel_client.signal_connect_change_factory                (slot (panel_slot_change_factory));

    if (!panel_initialize ())
        std::cerr << "SCIM Qt IM Module: Cannot connect to Panel!\n";
}

static void finalize (void)
{
    _focused_ic = 0;

    _default_instance.reset ();
    _fallback_instance.reset ();
    _fallback_factory.reset ();

#ifdef SCIM_HAS_CANDIDATES
    candidates_finalize ();
#endif
    panel_finalize ();

    _backend.reset ();
    _config.reset ();

    if (_config_module) {
        delete _config_module;
        _config_module = 0;
    }

    _scim_initialized = false;
}

/* -------------------------------------------------------------------------- */
/* ScimQtInputContext.                                                        */
ScimQtInputContext::ScimQtInputContext ()
    : impl (0)
{
    if (!_scim_initialized) {
        initialize ();
        _scim_initialized = true;
    }

    impl = new Impl;
    impl->id              = _context_count++;
    impl->preedit_caret   = 0;
    impl->is_on           = false;
    impl->use_preedit     = _on_the_spot;
    impl->preedit_started = false;
    impl->shared_si       = false;
    impl->cursor_x        = 0;
    impl->cursor_y        = 0;

    if (!_backend.null ()) {
        IMEngineFactoryPointer factory = _backend->get_default_factory (_language, "UTF-8");
        if (!factory.null ()) {
            impl->si = factory->create_instance ("UTF-8", _instance_count++);
            if (!impl->si.null ()) {
                impl->si->set_frontend_data (static_cast<void *> (this));
                attach_instance (impl->si);
            }
        }
    }

    if (_shared_input_method)
        impl->is_on = _config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), impl->is_on);

    _the_context = this;

    _panel_client.prepare (impl->id);
    _panel_client.register_input_context (impl->id,
                                          impl->si.null () ? String () : impl->si->get_factory_uuid ());
    set_ic_capabilities (this);
    _panel_client.send ();
}

ScimQtInputContext::~ScimQtInputContext ()
{
    if (impl) {
        if (_focused_ic == this)
            do_focus_out (this);

        _panel_client.prepare (impl->id);
        _panel_client.remove_input_context (impl->id);
        _panel_client.send ();

        impl->si.reset ();

        if (_the_context == this)
            _the_context = 0;

        delete impl;
        impl = 0;
    }
}

bool ScimQtInputContext::isValid () const
{
    return true;
}

void ScimQtInputContext::setFocusObject (QObject *object)
{
    if (!impl) return;

    impl->focus_object = object;

    if (object)
        do_focus_in (this);
    else
        do_focus_out (this);
}

void ScimQtInputContext::reset ()
{
    if (impl && !impl->si.null ()) {
        _panel_client.prepare (impl->id);
        impl->si->reset ();
        _panel_client.send ();
    }
}

void ScimQtInputContext::commit ()
{
    // Called before focus moves away: let the engine flush any pending state.
    if (impl && !impl->si.null ()) {
        _panel_client.prepare (impl->id);
        impl->si->reset ();
        _panel_client.send ();
    }
}

void ScimQtInputContext::update (Qt::InputMethodQueries queries)
{
    if (!impl || !(queries & Qt::ImCursorRectangle) || _focused_ic != this)
        return;

    QRectF r = QGuiApplication::inputMethod ()->cursorRectangle ();
    int x = (int) r.x ();
    int y = (int) (r.y () + r.height ());

    if (impl->cursor_x != x || impl->cursor_y != y) {
        impl->cursor_x = x;
        impl->cursor_y = y;
        // Window-local coords; candidates_show maps them per platform (absolute
        // on X11, parent-relative for the Wayland xdg_popup).
        _panel_client.prepare (impl->id);
        _panel_client.send ();
    }
}

bool ScimQtInputContext::filterEvent (const QEvent *event)
{
    if (!impl || impl->si.null ())
        return false;

    QEvent::Type type = event->type ();
    if (type != QEvent::KeyPress && type != QEvent::KeyRelease)
        return false;

    const QKeyEvent *qe = static_cast<const QKeyEvent *> (event);
    KeyEvent key = keyevent_qt_to_scim (qe);

    bool ret = false;
    _panel_client.prepare (impl->id);

    if (!filter_hotkeys (this, key)) {
        if (!impl->is_on || !impl->si->process_key_event (key))
            ret = _fallback_instance->process_key_event (key);
        else
            ret = true;
    } else {
        ret = true;
    }

    _panel_client.send ();
    return ret;
}

void ScimQtInputContext::invokeAction (QInputMethod::Action action, int cursorPosition)
{
    Q_UNUSED (action);
    Q_UNUSED (cursorPosition);
}

void ScimQtInputContext::sendEvent (QInputMethodEvent &event)
{
    if (impl && impl->focus_object)
        QCoreApplication::sendEvent (impl->focus_object, &event);
}

/* -------------------------------------------------------------------------- */
QPlatformInputContext *
ScimQtInputContextPlugin::create (const QString &key, const QStringList &paramList)
{
    Q_UNUSED (paramList);

    if (key.compare (QLatin1String ("scim"), Qt::CaseInsensitive) == 0)
        return new ScimQtInputContext;

    return 0;
}
