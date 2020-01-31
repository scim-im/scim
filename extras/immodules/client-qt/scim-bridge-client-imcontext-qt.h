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

#include <QApplication>
#include <QEvent>
#include <QFont>
#include <qpa/qplatforminputcontext.h>
#include <QInputMethodEvent>
#include <QObject>
#include <QPoint>
#include <QWidget>

#include "scim-bridge.h"
#include "scim-bridge-attribute.h"

#include "scim-bridge-client-imcontext.h"

#include "scim-bridge-client-common-qt.h"

/**
 * IMContext class for qt client.
 */
struct _ScimBridgeClientIMContext: public QPlatformInputContext
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
         * Filter a key event.
         *
         * @param event The key event.
         * @return If this event is consumed or not.
         */
        virtual bool filterEvent (const QEvent *event) = 0;

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
