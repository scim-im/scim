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
 * @brief This is the common header for qt client of scim-bridge.
 */


#ifndef SCIMBRIDGECOMMONQT_H_
#define SCIMBRIDGECOMMONQT_H_

#ifdef QT4
#include <QString>
#include <QStringList>
#else
#include <qstring.h>
#include <qstringlist.h>
#endif

#include "scim-bridge.h"

/**
 * The identifier name for SCIM input module.
 */
const QString SCIM_BRIDGE_IDENTIFIER_NAME = "scim-bridge";

#endif                                            /*SCIMBRIDGECOMMONQT_H_*/
