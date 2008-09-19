/** @file scim_private.cpp
 *  @brief Do some library initialize work here.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
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
 *
 * $Id: scim_private.cpp,v 1.6.4.1 2007/03/11 12:35:10 suzhe Exp $
 *
 */
#define Uses_C_STRING
#define Uses_C_STRING
#include "scim_private.h"
#include "scim.h"

#ifdef HAVE_LOCALE_H
  #include <locale.h>
#endif

#include <sys/time.h>

#ifdef TIME_WITH_SYS_TIME
  #include <time.h>
#endif

#include <stdlib.h>

namespace scim {

class TextdomainInitializer {
public:
    TextdomainInitializer () {

#ifdef HAVE_SETLOCALE
        char *locale = setlocale (LC_MESSAGES, 0);
        if (!locale || strcmp (locale, "C") == 0 || strcmp (locale, "POSIX") == 0)
            setlocale (LC_MESSAGES, "");
        locale = setlocale (LC_CTYPE, 0);
        if (!locale || strcmp (locale, "C") == 0 || strcmp (locale, "POSIX") == 0)
            setlocale (LC_CTYPE, "");
#endif

        bindtextdomain (GETTEXT_PACKAGE, SCIM_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    }
};

class RandSeedInitializer {
public:
    RandSeedInitializer () {
#ifdef HAVE_GETTIMEOFDAY
        timeval cur_time;
        if (gettimeofday (&cur_time, 0) == 0) {
            srand (cur_time.tv_sec);
        }
#else
        srand (time (0));
#endif
    }
};

static TextdomainInitializer __textdomain_initializer;
static RandSeedInitializer   __rand_seed_initializer;

} // namespace scim

/*
vi:ts=4:ai:nowrap:expandtab
*/
