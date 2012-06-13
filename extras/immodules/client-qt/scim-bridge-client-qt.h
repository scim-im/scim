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
 * @brief This is the header for qt client of scim-bridge.
 */

#ifndef SCIMBRIDGECLIENTQT_H_
#define SCIMBRIDGECLIENTQT_H_

#ifdef QT4
#include <QObject>
#include <QSocketNotifier>
#else
#include <qobject.h>
#include <qsocketnotifier.h>
#endif

#include "scim-bridge.h"
#include "scim-bridge-client-imcontext-qt.h"
#include "scim-bridge-client-common-qt.h"

/**
 * The public interface of scim-bridge-client for Qt apps.
 */
class ScimBridgeClientQt: public QObject
{

    Q_OBJECT

    public slots:

        void handle_message ();


    public:

        
        /**
         * Constructor.
         */
        ScimBridgeClientQt ();


        /**
         * Destructor.
         */
        ~ScimBridgeClientQt ();


        /**
         * A messenger is opened.
         */
        void messenger_opened ();


        /**
         * A messenger is closed.
         */
        void messenger_closed ();


    private:
        

        /**
         * The notifier for the messenger socket.
         */
        QSocketNotifier *socket_notifier;

};

#endif                                            /*SCIMBRIDGECLIENTQT_H_*/
