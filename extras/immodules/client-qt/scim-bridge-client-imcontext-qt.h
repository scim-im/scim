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

/**
 * @file
 * @author Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * @brief This is the header of the public interface of IMContexts.
 */

#ifndef SCIMBRIDGECLIENTIMCONTEXTQT_H_
#define SCIMBRIDGECLIENTIMCONTEXTQT_H_

#ifdef QT4
#include <QApplication>
#include <QEvent>
#include <QFont>
#include <QInputContext>
#include <QInputMethodEvent>
#include <QObject>
#include <QPoint>
#include <QWidget>
#else
#include <qapplication.h>
#include <qevent.h>
#include <qfont.h>
#include <qinputcontext.h>
#include <qobject.h>
#include <qptrlist.h>
#include <qpoint.h>
#include <qwidget.h>
#endif

#include "scim-bridge.h"
#include "scim-bridge-attribute.h"

#include "scim-bridge-client-imcontext.h"

#include "scim-bridge-client-common-qt.h"

/**
 * IMContext class for qt client.
 */
struct _ScimBridgeClientIMContext: public QInputContext
{

    public:

        /**
         * Initialize the class itself.
         */
        static void static_initialize ();
        
        /**
         * Finalize the class itself.
         */
        static void static_finalize ();

        /**
         * The connection with the agent is opened.
         */
        static void connection_opened ();

        /**
         * The connection with the agent is closed.
         */
        static void connection_closed ();

        /**
         * Allocate a new IMContext.
         *
         * @return A new IMContext.
         */
        static _ScimBridgeClientIMContext *alloc ();

        /**
         * Destructor.
         */
        virtual ~_ScimBridgeClientIMContext () {}

        /**
         * Filter a event from X11.
         *
         * @param widget The widget.
         * @param A event from X11.
         * @return If this event is consumed or not.
         */
        virtual bool x11FilterEvent (QWidget *widget, XEvent *event) = 0;

        /**
         * Filter a key event.
         *
         * @param event The key event.
         * @return If this event is consumed or not.
         */
        virtual bool filterEvent (const QEvent *event) = 0;

#ifdef QT4
        /**
         * The focus has been changed.
         */
        virtual void update () = 0;

        /**
         * Get the identifier name for this input context.
         *
         * @return The identifier name.
         */
        virtual QString identifierName () = 0;

        /**
         * Get the languages for the input context.
         *
         * @return The languages for the input context.
         */
        virtual QString language () = 0;

        /**
         * Filter a mouse event.
         *
         * @param offset The cursor offset in the preedit string.
         * @param event The mouse event. 
         */
        virtual void mouseHandler (int offset, QMouseEvent *event) = 0;

        /**
         * The current focused widget is destroied.
         *
         * @param widget The widget under destroying.
         */
        virtual void widgetDestroyed (QWidget *widget) = 0;

#else

        /**
         * Focus an IMContext.
         */
        virtual void setFocus () = 0;

        /**
         * Unfocus an IMContext.
         */
        virtual void unsetFocus () = 0;
        
        /**
         * Set the focused area in the display.
         *
         * @param x The X loation of the focused area.
         * @param y The Y loation of the focused area.
         * @param w The width of the focused area.
         * @param h The height of the focused area.
         * @param font The font.
         */
        virtual void setMicroFocus (int x, int y, int w, int h, QFont *font = 0) = 0;

        /**
         * Filter a mouse event.
         *
         * @param offset The cursor offset in the preedit string.
         * @param type The type of the event.
         * @param button The button of this event.
         * @param state The state of the button.
         */
        virtual void mouseHandler (int offset, QEvent::Type type, ButtonState button, ButtonState state) = 0;

#endif

        /**
         * Reset the current IME.
         */
        virtual void reset () = 0;

    protected:

        /**
         * Constructor.
         */
        _ScimBridgeClientIMContext () {}

};

#endif                                            /*SCIMBRIDGECLIENTIMCONTEXTQT_H_*/
