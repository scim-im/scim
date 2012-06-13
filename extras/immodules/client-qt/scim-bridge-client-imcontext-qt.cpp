/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <cassert>
#include <string>

#ifdef QT4
#include <QColor>
#include <QInputMethodEvent>
#include <QPalette>
#include <QTextCharFormat>

#ifdef Q_WS_X11
#include <QX11Info>
#endif
#endif

#include "scim-bridge-output.h"
#include "scim-bridge-string.h"

#include "scim-bridge-client.h"

#include "scim-bridge-client-imcontext-qt.h"
#include "scim-bridge-client-key-event-utility-qt.h"

#ifdef QT4
using namespace std;

using namespace Qt;

typedef QInputMethodEvent::Attribute QAttribute;
#endif

/* Static variables */
class ScimBridgeClientIMContextImpl;

static ScimBridgeClientIMContextImpl *focused_imcontext = NULL;

static bool key_event_forwarded = false;

/* Class Definition */
class ScimBridgeClientIMContextImpl: public _ScimBridgeClientIMContext
{

    public:

        ScimBridgeClientIMContextImpl ();
        ~ScimBridgeClientIMContextImpl ();

        bool x11FilterEvent (QWidget *widget, XEvent *event);
        bool filterEvent (const QEvent *event);

#ifdef QT4
        void update ();
        QString identifierName ();
        QString language ();

        void setFocusWidget (QWidget *widget);
        void widgetDestroyed (QWidget *widget);

        bool isComposing () const;
        void mouseHandler (int offset, QMouseEvent *event);
#else
        void setFocus ();
        void unsetFocus ();
        void setMicroFocus (int x, int y, int w, int h, QFont *font = 0);
        void mouseHandler (int offset, QEvent::Type type, ButtonState button, ButtonState state);
#endif

        void reset ();

        /* Semi private functions */
        void focus_in ();
        void focus_out ();

        void set_commit_string (const char *new_commit_string);
        void commit ();

        void forward_key_event (const ScimBridgeKeyEvent *key_event);

        void set_preedit_shown (bool shown);
        void set_preedit_string (const char *new_preedit_string);
        void set_preedit_attributes (ScimBridgeAttribute** const new_attributes, int new_attribute_count);
        void set_preedit_cursor_position (int new_cursor_position);
        void update_preedit ();

        scim_bridge_imcontext_id_t get_id () const;
        void set_id (scim_bridge_imcontext_id_t new_id);

#ifdef QT4
        bool get_surrounding_text (unsigned int before_max, unsigned int after_max, char **string, int *cursor_position);
        bool delete_surrounding_text (int offset, int length);
        bool replace_surrounding_text (const char *text, int cursor_position);
#endif

    private:

        scim_bridge_imcontext_id_t id;

        bool preedit_shown;

        QString preedit_string;

#ifdef QT4
        QList<QAttribute> preedit_attributes;
#else
        int preedit_selected_offset;
        int preedit_selected_length;
#endif

        int preedit_cursor_position;

        QString commit_string;

        QPoint cursor_location;

        void set_cursor_location (const QPoint &new_cursor_location);
};


/* Implementations */
void _ScimBridgeClientIMContext::static_initialize ()
{
}


void _ScimBridgeClientIMContext::static_finalize ()
{
}


void _ScimBridgeClientIMContext::connection_opened ()
{
}


void _ScimBridgeClientIMContext::connection_closed ()
{
}


_ScimBridgeClientIMContext *_ScimBridgeClientIMContext::alloc ()
{
    return new ScimBridgeClientIMContextImpl ();
}


ScimBridgeClientIMContextImpl::ScimBridgeClientIMContextImpl (): id (-1), preedit_shown (false)
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::ScimBridgeClientIMContextImpl ()");

#ifdef QT4
    preedit_attributes.push_back (QAttribute (QInputMethodEvent::Cursor, preedit_cursor_position, true, 0));
#else
    preedit_selected_offset = 0;
    preedit_selected_length = 0;
#endif

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is now down");
    } else if (scim_bridge_client_register_imcontext (this)) {
        scim_bridge_perrorln ("Failed to register the IMContext");
    } else {
        scim_bridge_pdebugln (1, "IMContext registered: id = %d", id);
    }
}


ScimBridgeClientIMContextImpl::~ScimBridgeClientIMContextImpl ()
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::~ScimBridgeClientIMContextImpl ()");

    if (this == focused_imcontext) focus_out ();

    if (!scim_bridge_client_is_messenger_opened ()) {
        scim_bridge_perrorln ("The messenger is now down");
    } else if (scim_bridge_client_deregister_imcontext (this)) {
        scim_bridge_perrorln ("Failed to deregister an IMContext");
    } else {
        scim_bridge_pdebugln (3, "IMContext deregistered: id = %d", id);
    }
}

#ifdef QT4

QString ScimBridgeClientIMContextImpl::identifierName ()
{
    return SCIM_BRIDGE_IDENTIFIER_NAME;
}

QString ScimBridgeClientIMContextImpl::language ()
{
    return "";
}

void ScimBridgeClientIMContextImpl::widgetDestroyed (QWidget *widget)
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::widgetDestroyed ()");
    focus_out ();
    update ();
}

void ScimBridgeClientIMContextImpl::setFocusWidget (QWidget *widget)
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::setFocusWidget ()");
    QInputContext::setFocusWidget (widget);
	if (widget == NULL) {
    	focus_out ();
	}
	else {
    	focus_in ();
	}
    update ();
}

void ScimBridgeClientIMContextImpl::update ()
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::update ()");

    QWidget *focused_widget = qApp->focusWidget ();
    if (focused_widget != NULL) {
        if (focused_imcontext == NULL)
            focus_in ();

        QRect rect = focused_widget->inputMethodQuery (ImMicroFocus).toRect ();
        QPoint point (rect.x (), rect.y () + rect.height ());
        QPoint new_cursor_location = focused_widget->mapToGlobal (point);
        set_cursor_location (new_cursor_location);
    }
}

bool ScimBridgeClientIMContextImpl::isComposing () const
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::isComposing ()");
    return preedit_string.size () > 0;
}

void ScimBridgeClientIMContextImpl::mouseHandler (int offset, QMouseEvent *mevent)
{
}

bool ScimBridgeClientIMContextImpl::get_surrounding_text (unsigned int before_max, unsigned int after_max, char **text, int *cursor_position)
{
    scim_bridge_pdebugln (6, "ScimBridgeClientIMContextImpl::get_surrounding_text ()");
    scim_bridge_perrorln ("FIXME: scim_bridge_client_imcontext_delete_surrounding_text () is not yet implemented.");
    return false;
}

bool ScimBridgeClientIMContextImpl::delete_surrounding_text (int offset, int length)
{
    scim_bridge_pdebugln (6, "ScimBridgeClientIMContextImpl::delete_surrounding_text ()");
    scim_bridge_perrorln ("FIXME: scim_bridge_client_imcontext_delete_surrounding_text () is not yet implemented.");
    return false;
}

bool ScimBridgeClientIMContextImpl::replace_surrounding_text (const char *text, int cursor_position)
{
    scim_bridge_pdebugln (6, "ScimBridgeClientIMContextImpl::replace_surrounding_text ()");
    scim_bridge_perrorln ("FIXME: scim_bridge_client_imcontext_delete_surrounding_text () is not yet implemented.");
    return false;
}

#else

void ScimBridgeClientIMContextImpl::setFocus ()
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::setFocus ()");
    
    focus_in ();
}


void ScimBridgeClientIMContextImpl::unsetFocus ()
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::unsetFocus ()");
    
    focus_out ();
}


void ScimBridgeClientIMContextImpl::setMicroFocus (int x, int y, int w, int h, QFont *qfont)
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::setMicroFocus ()");

    QPoint new_cursor_location (x, y + h);
    set_cursor_location (new_cursor_location);
}

void ScimBridgeClientIMContextImpl::mouseHandler (int offset, QEvent::Type type, ButtonState button, ButtonState state)
{
}
#endif


bool ScimBridgeClientIMContextImpl::x11FilterEvent (QWidget *widget, XEvent *xevent)
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::x11FilterEvent ()");

#ifdef Q_WS_X11
    if (key_event_forwarded || (xevent->type != XKeyPress && xevent->type != XKeyRelease)) return false;
    
    if (focused_imcontext != this) focus_in ();

    if (scim_bridge_client_is_messenger_opened ()) {
        ScimBridgeKeyEvent *bridge_key_event = scim_bridge_key_event_x11_to_bridge (xevent);

        boolean consumed = FALSE;
        const retval_t retval_error = scim_bridge_client_handle_key_event (this, bridge_key_event, &consumed);

        scim_bridge_free_key_event (bridge_key_event);

        if (retval_error) {
            scim_bridge_perrorln ("An IOException at x11FilterEvent ()");
        } else {
            return consumed;
        }
    }
#endif

    return false;
}


bool ScimBridgeClientIMContextImpl::filterEvent (const QEvent *qevent)
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::filterEvent ()");

#ifndef Q_WS_X11
    if (key_event_forwarded || (qevent->type () != QEvent::KeyPress && qevent->type () != QEvent::KeyRelease)) return false;
    
    if (focused_imcontext != this) focus_in ();

    if (scim_bridge_client_is_messenger_opened ()) {
        const QKeyEvent *key_event = static_cast<const QKeyEvent*> (qevent);
        ScimBridgeKeyEvent *bridge_key_event = scim_bridge_key_event_qt_to_bridge (key_event);
        
        if (scim_bridge_key_event_get_code (bridge_key_event) == SCIM_BRIDGE_KEY_CODE_NullKey)
            return false;
        
        boolean consumed = FALSE;
        const retval_t retval_error = scim_bridge_client_handle_key_event (this, bridge_key_event, &consumed);

        scim_bridge_free_key_event (bridge_key_event);

        if (retval_error) {
            scim_bridge_perrorln ("An IOException at filterEvent ()");
        } else {
            return consumed;
        }
    }
#endif

    return false;
}


void ScimBridgeClientIMContextImpl::reset ()
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::reset ()");

    preedit_cursor_position = 0;
#ifdef QT4
    preedit_attributes.clear ();
    preedit_attributes.push_back (QAttribute (QInputMethodEvent::Cursor, preedit_cursor_position, true, 0));
#else
    preedit_selected_offset = 0;
    preedit_selected_length = 0;
#endif
    preedit_string = "";

    if (scim_bridge_client_is_messenger_opened ()) {
        if (scim_bridge_client_reset_imcontext (this)) {
            scim_bridge_perrorln ("An IOException at filterEvent ()");
        }
    }

#ifndef QT4
    QInputContext::reset ();
#endif
}


/* Private Functions */
void ScimBridgeClientIMContextImpl::set_cursor_location (const QPoint &new_cursor_location)
{
    scim_bridge_pdebugln (4, "ScimBridgeClientIMContextImpl::set_cursor_location ()");
    
    if (cursor_location != new_cursor_location) {
        cursor_location = new_cursor_location;

        scim_bridge_pdebugln (3, "The cursor location is changed: x = %d\ty = %d", cursor_location.x (), cursor_location.y ());

        if (scim_bridge_client_is_messenger_opened ()) {
            if (scim_bridge_client_set_cursor_location (this, cursor_location.x (), cursor_location.y ())) scim_bridge_perrorln ("An IOException occurred at set_cursor_location ()");
        }
    }
}


void ScimBridgeClientIMContextImpl::focus_in ()
{
    scim_bridge_pdebugln (8, "ScimBridgeClientIMContextImpl::focus_in ()");

    if (focused_imcontext != NULL && focused_imcontext != this) focused_imcontext->focus_out ();

    focused_imcontext = this;

    if (!scim_bridge_client_is_messenger_opened () && scim_bridge_client_is_reconnection_enabled ()) {
        scim_bridge_pdebugln (7, "Trying to open the connection again...");
        scim_bridge_client_open_messenger ();
    }

    if (scim_bridge_client_is_messenger_opened ()) {
        if (scim_bridge_client_change_focus (this, TRUE)) {
            scim_bridge_perrorln ("An IOException occurred at focus_in ()");
        }
    }
}



void ScimBridgeClientIMContextImpl::focus_out ()
{
    scim_bridge_pdebugln (8, "ScimBridgeClientIMContextImpl::focus_out ()");

    if (focused_imcontext != this) return;

    if (scim_bridge_client_is_messenger_opened ()) {
        if (scim_bridge_client_change_focus (this, false)) {
            scim_bridge_perrorln ("An IOException occurred at focus_out ()");
        }
    }

	if (preedit_shown) {
    	set_preedit_shown (false);
    	update_preedit ();
	}

    focused_imcontext = NULL;
}


void ScimBridgeClientIMContextImpl::set_commit_string (const char *new_string)
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::set_commit_string ()");
    
    commit_string = QString::fromUtf8 (new_string);
}


void ScimBridgeClientIMContextImpl::commit ()
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::commit ()");
    
    if (commit_string.length () <= 0) return;
#ifdef QT4
    scim_bridge_pdebugln (9, "commit string: %s", commit_string.toUtf8 ().data ());
#endif

    const bool is_composing = isComposing ();

#ifdef QT4
    QInputMethodEvent commit_event;
    commit_event.setCommitString (commit_string);
    sendEvent (commit_event);
#else
    if (!is_composing) sendIMEvent (QEvent::IMStart);
    sendIMEvent (QEvent::IMEnd, commit_string);
#endif

    if (is_composing) update_preedit ();
}


void ScimBridgeClientIMContextImpl::forward_key_event (const ScimBridgeKeyEvent *key_event)
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::forward_key_event ()");

    QWidget *focused_widget = qApp->focusWidget ();
    if (focused_widget != NULL) {
        key_event_forwarded = true;
#ifdef Q_WS_X11
        const WId window_id = focused_widget->winId ();
#ifdef QT4
        Display *x11_display = QX11Info::display();
#else
        Display *x11_display = qt_xdisplay ();
#endif
        XEvent *x_event = scim_bridge_key_event_bridge_to_x11 (key_event, x11_display, window_id);
        qApp->x11ProcessEvent (x_event);
        free (x_event);
#else
        QKeyEvent *forwarded_key_event = scim_bridge_key_event_bridge_to_qt (key_event);
        QApplication::sendEvent (focused_widget, forwarded_key_event);
        delete forwarded_key_event;
#endif
        key_event_forwarded = false;
    } else {
        scim_bridge_pdebugln (4, "No widget is focused");
    }
}


void ScimBridgeClientIMContextImpl::set_preedit_shown (bool shown) 
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::set_preedit_shown ()");
    
    preedit_shown = shown;
    if (!preedit_shown) {
        preedit_string = "";
        preedit_cursor_position = 0;
#ifdef QT4
        preedit_attributes.clear ();
        preedit_attributes.push_back (QAttribute (QInputMethodEvent::Cursor, preedit_cursor_position, true, 0));
#else
        preedit_selected_offset = 0;
        preedit_selected_length = 0;
#endif
    }
}

void ScimBridgeClientIMContextImpl::set_preedit_cursor_position (int new_cursor_position)
{   
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::set_preedit_cursor_position ()");
    
    preedit_cursor_position = new_cursor_position;
}

void ScimBridgeClientIMContextImpl::set_preedit_string (const char *new_string) 
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::set_preedit_string ()");
    
    preedit_string = QString::fromUtf8 (new_string);
}


void ScimBridgeClientIMContextImpl::set_preedit_attributes (ScimBridgeAttribute** const attributes, int attribute_count) 
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::set_preedit_attribute ()");
    
#ifdef QT4
    preedit_attributes.clear ();
    preedit_attributes.push_back (QAttribute (QInputMethodEvent::Cursor, preedit_cursor_position, true, 0));
#else
    preedit_selected_offset = 0;
    preedit_selected_length = 0;
#endif

    for (int i = 0; i < attribute_count; ++i) {
        const ScimBridgeAttribute *attribute = attributes[i];
        
        const size_t attribute_begin = scim_bridge_attribute_get_begin (attribute);
        const size_t attribute_end = scim_bridge_attribute_get_end (attribute);
        const scim_bridge_attribute_type_t attribute_type = scim_bridge_attribute_get_type (attribute);
        const scim_bridge_attribute_value_t attribute_value = scim_bridge_attribute_get_value (attribute);

#ifdef QT4
        const size_t attribute_length = attribute_end - attribute_begin;

        const QWidget *focused_widget = qApp->focusWidget ();
        const QPalette &palette = focused_widget->palette ();

        const QBrush &reversed_foreground = palette.base ();
        const QBrush &reversed_background = palette.text ();
        const QBrush &hilight_foreground = palette.highlightedText ();
        const QBrush &hilight_background = palette.highlight ();

        switch (attribute_type) {
        case ATTRIBUTE_DECORATE:
            switch (attribute_value) {
            case SCIM_BRIDGE_ATTRIBUTE_DECORATE_HIGHLIGHT:
                {
                    QTextCharFormat text_format;
                    text_format.setForeground (hilight_foreground);
                    text_format.setBackground (hilight_background);
                    QAttribute qt_attribute (QInputMethodEvent::TextFormat, attribute_begin, attribute_length, text_format);
                    preedit_attributes.push_back (qt_attribute);
                }
                break;
            case SCIM_BRIDGE_ATTRIBUTE_DECORATE_REVERSE:
                {
                    QTextCharFormat text_format;
                    text_format.setForeground (reversed_foreground);
                    text_format.setBackground (reversed_background);
                    QAttribute qt_attribute (QInputMethodEvent::TextFormat, attribute_begin, attribute_length, text_format);
                    preedit_attributes.push_back (qt_attribute);
                }
                break;
            case SCIM_BRIDGE_ATTRIBUTE_DECORATE_UNDERLINE:
                {
                    QTextCharFormat text_format;
                    text_format.setProperty (QTextFormat::FontUnderline, true);
                    QAttribute qt_attribute (QInputMethodEvent::TextFormat, attribute_begin, attribute_length, text_format);
                    preedit_attributes.push_back (qt_attribute);
                }
                break;
            }
            break;
        case ATTRIBUTE_FOREGROUND:
            {
                QTextCharFormat text_format;
                const unsigned int red = scim_bridge_attribute_get_red (attribute);
                const unsigned int green = scim_bridge_attribute_get_green (attribute);
                const unsigned int blue = scim_bridge_attribute_get_blue (attribute);
                text_format.setForeground (QColor (red, green, blue));
                QAttribute qt_attribute (QInputMethodEvent::TextFormat, attribute_begin, attribute_length, text_format);
                preedit_attributes.push_back (qt_attribute);
            }
            break;
        case ATTRIBUTE_BACKGROUND:
            {
                QTextCharFormat text_format;
                const unsigned int red = scim_bridge_attribute_get_red (attribute);
                const unsigned int green = scim_bridge_attribute_get_green (attribute);
                const unsigned int blue = scim_bridge_attribute_get_blue (attribute);
                text_format.setBackground (QColor (red, green, blue));
                QAttribute qt_attribute (QInputMethodEvent::TextFormat, attribute_begin, attribute_length, text_format);
                preedit_attributes.push_back (qt_attribute);
            }
            break;
        default:
            break;
        }
#else
        if (attribute_type == ATTRIBUTE_DECORATE && (attribute_value == SCIM_BRIDGE_ATTRIBUTE_DECORATE_HIGHLIGHT || attribute_value == SCIM_BRIDGE_ATTRIBUTE_DECORATE_REVERSE)) {
            preedit_selected_offset = attribute_begin;
            preedit_selected_length = attribute_end - attribute_begin;
            break;
        }
#endif
    }
}


void ScimBridgeClientIMContextImpl::update_preedit ()
{
    scim_bridge_pdebugln (5, "ScimBridgeClientIMContextImpl::update_preedit ()");

#ifdef QT4
    preedit_attributes[0] = QAttribute (QInputMethodEvent::Cursor, preedit_cursor_position, true, 0);
    QInputMethodEvent im_event (preedit_string, preedit_attributes);
    sendEvent (im_event);
    update ();
#else
    if (preedit_shown) {
        if (!isComposing ()) sendIMEvent (QEvent::IMStart);
        const size_t preedit_length = preedit_string.length ();

        size_t cursor_position = preedit_cursor_position;
        if (cursor_position > preedit_length) cursor_position = preedit_length;

        size_t selected_length = preedit_selected_length;
        if (cursor_position + selected_length > preedit_length) selected_length = preedit_length - cursor_position;
        sendIMEvent (QEvent::IMCompose, preedit_string, cursor_position, selected_length);
    } else {
        if (isComposing ()) sendIMEvent (QEvent::IMEnd);
    }
#endif
}


scim_bridge_imcontext_id_t ScimBridgeClientIMContextImpl::get_id () const
{
    return id;
}


void ScimBridgeClientIMContextImpl::set_id (scim_bridge_imcontext_id_t new_id)
{
    id = new_id;
}


/* Bindings */
scim_bridge_imcontext_id_t scim_bridge_client_imcontext_get_id (const ScimBridgeClientIMContext *imcontext)
{
    const ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<const ScimBridgeClientIMContextImpl*> (imcontext);
    return imcontext_impl->get_id ();
}


void scim_bridge_client_imcontext_set_id (ScimBridgeClientIMContext *imcontext, scim_bridge_imcontext_id_t new_id)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->set_id (new_id);
}

void scim_bridge_client_imcontext_beep (ScimBridgeClientIMContext *imcontext)
{
    QApplication::beep ();
}


void scim_bridge_client_imcontext_focus_in (ScimBridgeClientIMContext *imcontext)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->focus_in ();
}


void scim_bridge_client_imcontext_focus_out (ScimBridgeClientIMContext *imcontext)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->focus_out ();
}


void scim_bridge_client_imcontext_set_commit_string (ScimBridgeClientIMContext *imcontext, const char *commit_string)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->set_commit_string (commit_string);
}


void scim_bridge_client_imcontext_commit (ScimBridgeClientIMContext *imcontext)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->commit ();
}


void scim_bridge_client_imcontext_forward_key_event (ScimBridgeClientIMContext *imcontext, const ScimBridgeKeyEvent *key_event)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->forward_key_event (key_event);
}


void scim_bridge_client_imcontext_set_preedit_shown (ScimBridgeClientIMContext *imcontext, boolean shown)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->set_preedit_shown (shown);
}


void scim_bridge_client_imcontext_set_preedit_string (ScimBridgeClientIMContext *imcontext, const char *preedit_string)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->set_preedit_string (preedit_string);
}


void scim_bridge_client_imcontext_set_preedit_cursor_position (ScimBridgeClientIMContext *imcontext, int cursor_position)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->set_preedit_cursor_position (cursor_position);
}



void scim_bridge_client_imcontext_set_preedit_attributes (ScimBridgeClientIMContext *imcontext, ScimBridgeAttribute** const attributes, int attribute_count)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->set_preedit_attributes (attributes, attribute_count);
}


void scim_bridge_client_imcontext_update_preedit (ScimBridgeClientIMContext *imcontext)
{
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    imcontext_impl->update_preedit ();
}


boolean scim_bridge_client_imcontext_get_surrounding_text (ScimBridgeClientIMContext *imcontext, int before_max, int after_max, char **string, int *cursor_position)
{
#ifdef QT4
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    return imcontext_impl->get_surrounding_text (before_max, after_max, string, cursor_position);
#else
    scim_bridge_perrorln ("FIXME: scim_bridge_client_imcontext_get_surrounding_text () is not yet implemented.");
    return FALSE;
#endif
}


boolean scim_bridge_client_imcontext_delete_surrounding_text (ScimBridgeClientIMContext *imcontext, int offset, int length)
{
#ifdef QT4
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    return imcontext_impl->delete_surrounding_text (offset, length);
#else
    scim_bridge_perrorln ("FIXME: scim_bridge_client_imcontext_delete_surrounding_text () is not yet implemented.");
    return FALSE;
#endif
}


boolean scim_bridge_client_imcontext_replace_surrounding_text (ScimBridgeClientIMContext *imcontext, int cursor_position, const char *string)
{
#ifdef QT4
    ScimBridgeClientIMContextImpl *imcontext_impl = static_cast<ScimBridgeClientIMContextImpl*> (imcontext);
    return imcontext_impl->replace_surrounding_text (string, cursor_position);
#else
    scim_bridge_perrorln ("FIXME: scim_bridge_client_imcontext_replace_surrounding_text () is not yet implemented.");
    return FALSE;
#endif
}


void scim_bridge_client_imcontext_imengine_status_changed (ScimBridgeClientIMContext *imcontext, boolean enabled)
{
    // Do nothing.
}

