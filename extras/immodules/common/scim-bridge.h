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
 * @brief This header describes about fundamental definitions of scim-bridge.
 */

#ifndef SCIMBRIDE_H_
#define SCIMBRIDE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifndef SCIM_VERSION
#define SCIM_VERSION "<unknown>"
#endif

/**
 * The type of boolean.
 */
typedef int boolean;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/**
 * The type of retvals.
 */
typedef int retval_t;

/**
 * The return value of failture.
 */
#define RETVAL_FAILED -1

/**
 * The return value of successness.
 */
#define RETVAL_SUCCEEDED 0

/**
 * The return value of ignoreness.
 */
#define RETVAL_IGNORED 1
#endif                                            /*SCIMBRIDE_H_*/
