/** @file scim_x11_ic.cpp
 * implementation of class X11ICManager.
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
 * $Id: scim_x11_ic.cpp,v 1.19 2005/06/26 16:35:12 suzhe Exp $
 *
 */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "IMdkit/IMdkit.h"
#include "IMdkit/Xi18n.h"

#include "scim_private.h"
#include "scim.h"
#include "scim_x11_ic.h"

using namespace scim;

static int
_is_attr (const char *attr, XICAttribute * attr_list)
{
  return !strcmp (attr, attr_list->name);
}

X11ICManager::X11ICManager ()
    : m_ic_list (NULL),
      m_free_list (NULL)
{
}

X11ICManager::~X11ICManager ()
{
    X11IC *it;

    it = m_ic_list;
    while (it != NULL) {
        m_ic_list = it->next;
        delete it;
        it = m_ic_list;
    }

    it = m_free_list;
    while (it != NULL) {
        m_free_list = it->next;
        delete it;
        it = m_free_list;
    }
}

X11IC *
X11ICManager::new_ic ()
{
    static CARD16 base_icid = 1;
    X11IC *rec;
    
    if (m_free_list != NULL) {
        rec = m_free_list;
        m_free_list = m_free_list->next;
    } else {
        rec = new X11IC;
    }

    if (base_icid == 0) base_icid = 1;

    rec->icid = base_icid ++;

    rec->next = m_ic_list;
    m_ic_list = rec;
    return rec;
}

void
X11ICManager::delete_ic (CARD16 icid)
{
    X11IC *rec, *last = NULL;

    for (rec = m_ic_list; rec != NULL; last = rec, rec = rec->next) {
        if (rec->icid == icid) {
            if (last != NULL)
                last->next = rec->next;
            else
                m_ic_list = rec->next;

            rec->next = m_free_list;
            m_free_list = rec;

            rec->siid = -1;
            rec->icid = 0;
            rec->connect_id = 0;
            rec->client_win = 0;
            rec->focus_win = 0;
            rec->shared_siid = false;
            rec->xims_on = false;
            rec->encoding = String ();
            rec->locale = String ();

            return;
        }
    }
    return;
}

String
X11ICManager::get_connection_locale (CARD16 connect_id)
{
    ConnectionLocaleMap::iterator it =
        m_connect_locales.find ((int)connect_id);

    if (it != m_connect_locales.end ())
        return it->second;

    return String ();
}

void
X11ICManager::new_connection (IMOpenStruct *call_data)
{
    if (call_data == NULL) return;

    String locale = scim_validate_locale (String (call_data->lang.name));

    if (!locale.length ()) {
        locale = String ("C");
    }

    m_connect_locales [(int)call_data->connect_id] = locale;
}

void
X11ICManager::delete_connection (IMCloseStruct *call_data)
{
    if (call_data == NULL) return;

    m_connect_locales.erase ((int)call_data->connect_id);
}

uint32
X11ICManager::create_ic (IMChangeICStruct *call_data, int siid)
{
    X11IC * rec;
    
    rec = new_ic ();
    if (rec == NULL) return 0;

    call_data->icid = rec->icid;
    rec->connect_id = call_data->connect_id;
    rec->siid = siid;
    rec->shared_siid = false;
    rec->xims_on = false;
    rec->onspot_preedit_started = false;
    rec->onspot_preedit_length = 0;
    rec->onspot_caret = 0;
    rec->focus_win = (Window) 0;
    rec->client_win = (Window) 0;
    rec->input_style = 0;
    rec->pre_attr.spot_location.x = -1;
    rec->pre_attr.spot_location.y = -1;

    return store_ic_values (rec, call_data);
}

X11IC *
X11ICManager::find_ic (CARD16 icid)
{
    X11IC *rec = m_ic_list;
    while (rec != NULL) {
        if (rec->icid == icid)
            return rec;
        rec = rec->next;
    }
    return NULL;
}

X11IC *
X11ICManager::find_ic_by_siid (int siid)
{
    X11IC *rec = m_ic_list;
    while (rec != NULL) {
        if (rec->siid == siid)
            return rec;
        rec = rec->next;
    }
    return NULL;
}

void
X11ICManager::destroy_ic (IMDestroyICStruct *call_data)
{
    if (call_data == NULL) return;

    delete_ic (call_data->icid);
}

uint32 
X11ICManager::store_ic_values (X11IC *rec, IMChangeICStruct *call_data)
{
    XICAttribute *ic_attr = call_data->ic_attr;
    XICAttribute *pre_attr = call_data->preedit_attr;
    XICAttribute *sts_attr = call_data->status_attr;

    int i;
    uint32 attrs = 0;

    // Set main attributes
    for (i=0; i< (int) call_data->ic_attr_num; ++i, ++ic_attr) {
        if (_is_attr (XNInputStyle, ic_attr)) {
            rec->input_style = * (INT32 *) ic_attr->value;
            attrs |= SCIM_X11_IC_INPUT_STYLE;
        } else if (_is_attr (XNClientWindow, ic_attr)) {
            rec->client_win = *(Window *) ic_attr->value;
            attrs |= SCIM_X11_IC_CLIENT_WINDOW;
        } else if (_is_attr (XNFocusWindow, ic_attr)) {
            rec->focus_win = *(Window *) ic_attr->value;
            attrs |= SCIM_X11_IC_FOCUS_WINDOW;
        } else {
            std::cerr << __FILE__ << "(" << __LINE__ << "):"
                 <<"Unknown attr: " << ic_attr->name << std::endl;
        }
    }

    // Set preedit attributes
    for (i = 0; i < (int) call_data->preedit_attr_num; ++i, ++pre_attr) {
        if (_is_attr (XNArea, pre_attr)) {
            rec->pre_attr.area = *(XRectangle *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_AREA;
        } else if (_is_attr (XNAreaNeeded, pre_attr)) {
            rec->pre_attr.area_needed = *(XRectangle *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_AREA_NEEDED;
        } else if (_is_attr (XNSpotLocation, pre_attr)) {
            rec->pre_attr.spot_location = *(XPoint *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_SPOT_LOCATION;
        } else if (_is_attr (XNColormap, pre_attr)) {
            rec->pre_attr.cmap = *(Colormap *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_COLORMAP;
        } else if (_is_attr (XNStdColormap, pre_attr)) {
            rec->pre_attr.cmap = *(Colormap *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_COLORMAP;
        } else if (_is_attr (XNForeground, pre_attr)) {
            rec->pre_attr.foreground = *(CARD32 *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_FOREGROUND;
        } else if (_is_attr (XNBackground, pre_attr)) {
            rec->pre_attr.background = *(CARD32 *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_BACKGROUND;
        } else if (_is_attr (XNBackgroundPixmap, pre_attr)) {
            rec->pre_attr.bg_pixmap = *(Pixmap *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_BG_PIXMAP;
        } else if (_is_attr (XNFontSet, pre_attr)) {
            rec->pre_attr.base_font = (char*) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_FONTSET;
        } else if (_is_attr (XNLineSpace, pre_attr)) {
            rec->pre_attr.line_space = *(CARD32 *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_LINE_SPACE;
        } else if (_is_attr (XNCursor, pre_attr)) {
            rec->pre_attr.cursor = *(Cursor *) pre_attr->value;
            attrs |= SCIM_X11_IC_PRE_CURSOR;
        } else {
            std::cerr << __FILE__ << "(" << __LINE__ << "):"
                 <<"Unknown attr: " << pre_attr->name << std::endl;
        }
    }

    // Set status attributes
    for (i = 0; i < (int) call_data->status_attr_num; ++i, ++sts_attr) {
        if (_is_attr (XNArea, sts_attr)) {
            rec->sts_attr.area = *(XRectangle *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_AREA;
        } else if (_is_attr (XNAreaNeeded, sts_attr)) {
            rec->sts_attr.area_needed = *(XRectangle *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_AREA_NEEDED;
        } else if (_is_attr (XNColormap, sts_attr)) {
            rec->sts_attr.cmap = *(Colormap *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_COLORMAP;
        } else if (_is_attr (XNStdColormap, sts_attr)) {
            rec->sts_attr.cmap = *(Colormap *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_COLORMAP;
        } else if (_is_attr (XNForeground, sts_attr)) {
            rec->sts_attr.foreground = *(CARD32 *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_FOREGROUND;
        } else if (_is_attr (XNBackground, sts_attr)) {
            rec->sts_attr.background = *(CARD32 *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_BACKGROUND;
        } else if (_is_attr (XNBackgroundPixmap, sts_attr)) {
            rec->sts_attr.bg_pixmap = *(Pixmap *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_BG_PIXMAP;
        } else if (_is_attr (XNFontSet, sts_attr)) {
            rec->sts_attr.base_font = (char*) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_FONTSET;
        } else if (_is_attr (XNLineSpace, sts_attr)) {
            rec->sts_attr.line_space = *(CARD32 *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_LINE_SPACE;
        } else if (_is_attr (XNCursor, sts_attr)) {
            rec->sts_attr.cursor = *(Cursor *) sts_attr->value;
            attrs |= SCIM_X11_IC_STS_CURSOR;
        } else {
            std::cerr << __FILE__ << "(" << __LINE__ << "):"
                 <<"Unknown attr: " << sts_attr->name << std::endl;
        }
    }

    String connect_locale =
        get_connection_locale ((int)call_data->connect_id);

    if (connect_locale != rec->locale) {
        rec->locale = connect_locale;
        rec->encoding = scim_get_locale_encoding (connect_locale);
        attrs |= SCIM_X11_IC_ENCODING;
    }

    return attrs;
}

uint32 
X11ICManager::set_ic_values (IMChangeICStruct *call_data)
{
    if (call_data == NULL) return 0;
    
    X11IC *rec = find_ic (call_data->icid);

    if (rec == NULL) return 0;

    return store_ic_values (rec, call_data);
}

uint32
X11ICManager::get_ic_values (IMChangeICStruct *call_data)
{
    if (call_data == NULL) return 0;
    
    XICAttribute *ic_attr = call_data->ic_attr;
    XICAttribute *pre_attr = call_data->preedit_attr;
    XICAttribute *sts_attr = call_data->status_attr;

    X11IC *rec = find_ic (call_data->icid);

    if (rec == NULL) return 0;

    int i;
    uint32 attrs = 0;

    for (i = 0; i < (int) call_data->ic_attr_num; ++i, ++ic_attr) {
        if (_is_attr (XNFilterEvents, ic_attr)) {
            ic_attr->value = (void *) malloc (sizeof (CARD32));
            *(CARD32 *) ic_attr->value = KeyPressMask | KeyReleaseMask;
            ic_attr->value_length = sizeof (CARD32);
            attrs |= SCIM_X11_IC_FILTER_EVENTS;
        } else {
            std::cerr << __FILE__ << "(" << __LINE__ << "):"
                 <<"Unknown attr: " << ic_attr->name << std::endl;
        }
    }

    // preedit attributes
    for (i = 0; i < (int) call_data->preedit_attr_num; ++i, ++pre_attr) {
        if (_is_attr (XNArea, pre_attr)) {
            pre_attr->value = (void *) malloc (sizeof (XRectangle));
            *(XRectangle *) pre_attr->value = rec->pre_attr.area;
            pre_attr->value_length = sizeof (XRectangle);
            attrs |= SCIM_X11_IC_PRE_AREA;
        } else if (_is_attr (XNAreaNeeded, pre_attr)) {
            pre_attr->value = (void *) malloc (sizeof (XRectangle));
            *(XRectangle *) pre_attr->value = rec->pre_attr.area_needed;
            pre_attr->value_length = sizeof (XRectangle);
            attrs |= SCIM_X11_IC_PRE_AREA_NEEDED;
        } else if (_is_attr (XNSpotLocation, pre_attr)) {
            pre_attr->value = (void *) malloc (sizeof (XPoint));
            *(XPoint *) pre_attr->value = rec->pre_attr.spot_location;
            pre_attr->value_length = sizeof (XPoint);
            attrs |= SCIM_X11_IC_PRE_SPOT_LOCATION;
        } else if (_is_attr (XNFontSet, pre_attr)) {
            CARD16 base_len = (CARD16) rec->pre_attr.base_font.size ();
            int total_len = sizeof (CARD16) + (CARD16) base_len;
            char *p;
            pre_attr->value = (void *) malloc (total_len);
            p = (char *) pre_attr->value;
            memmove (p, &base_len, sizeof (CARD16));
            p += sizeof (CARD16);
            strncpy (p, rec->pre_attr.base_font.c_str (), base_len);
            pre_attr->value_length = total_len;
            attrs |= SCIM_X11_IC_PRE_FONTSET;
        } else if (_is_attr (XNForeground, pre_attr)) {
            pre_attr->value = (void *) malloc (sizeof (CARD32));
            *(CARD32 *) pre_attr->value = rec->pre_attr.foreground;
            pre_attr->value_length = sizeof (CARD32);
            attrs |= SCIM_X11_IC_PRE_FOREGROUND;
        } else if (_is_attr (XNBackground, pre_attr)) {
            pre_attr->value = (void *) malloc (sizeof (CARD32));
            *(CARD32 *) pre_attr->value = rec->pre_attr.background;
            pre_attr->value_length = sizeof (CARD32);
            attrs |= SCIM_X11_IC_PRE_BACKGROUND;
        } else if (_is_attr (XNLineSpace, pre_attr)) {
            pre_attr->value = (void *) malloc (sizeof (CARD32));
            *(CARD32 *) pre_attr->value = rec->pre_attr.line_space;
            pre_attr->value_length = sizeof (CARD32);
            attrs |= SCIM_X11_IC_PRE_LINE_SPACE;
        } else {
            std::cerr << __FILE__ << "(" << __LINE__ << "):"
                 <<"Unknown attr: " << pre_attr->name << std::endl;
        }
    }

    // status attributes
    for (i = 0; i < (int) call_data->status_attr_num; ++i, ++sts_attr) {
        if (_is_attr (XNArea, sts_attr)) {
            sts_attr->value = (void *) malloc (sizeof (XRectangle));
            *(XRectangle *) sts_attr->value = rec->sts_attr.area;
            sts_attr->value_length = sizeof (XRectangle);
            attrs |= SCIM_X11_IC_STS_AREA;
        } else if (_is_attr (XNAreaNeeded, sts_attr)) {
            sts_attr->value = (void *) malloc (sizeof (XRectangle));
            *(XRectangle *) sts_attr->value = rec->sts_attr.area_needed;
            sts_attr->value_length = sizeof (XRectangle);
            attrs |= SCIM_X11_IC_STS_AREA_NEEDED;
        } else if (_is_attr (XNFontSet, sts_attr)) {
            CARD16 base_len = (CARD16) rec->sts_attr.base_font.size ();
            int total_len = sizeof (CARD16) + (CARD16) base_len;
            char *p;
            sts_attr->value = (void *) malloc (total_len);
            p = (char *) sts_attr->value;
            memmove (p, &base_len, sizeof (CARD16));
            p += sizeof (CARD16);
            strncpy (p, rec->sts_attr.base_font.c_str (), base_len);
            sts_attr->value_length = total_len;
            attrs |= SCIM_X11_IC_STS_FONTSET;
        } else if (_is_attr (XNForeground, sts_attr)) {
            sts_attr->value = (void *) malloc (sizeof (CARD32));
            *(CARD32 *) sts_attr->value = rec->sts_attr.foreground;
            sts_attr->value_length = sizeof (CARD32);
            attrs |= SCIM_X11_IC_STS_FOREGROUND;
        } else if (_is_attr (XNBackground, sts_attr)) {
            sts_attr->value = (void *) malloc (sizeof (CARD32));
            *(CARD32 *) sts_attr->value = rec->sts_attr.background;
            sts_attr->value_length = sizeof (CARD32);
            attrs |= SCIM_X11_IC_STS_BACKGROUND;
        } else if (_is_attr (XNLineSpace, sts_attr)) {
            sts_attr->value = (void *) malloc (sizeof (CARD32));
            *(CARD32 *) sts_attr->value = rec->sts_attr.line_space;
            sts_attr->value_length = sizeof (CARD32);
            attrs |= SCIM_X11_IC_STS_LINE_SPACE;
        } else {
            std::cerr << __FILE__ << "(" << __LINE__ << "):"
                 <<"Unknown attr: " << sts_attr->name << std::endl;
        }
    }

    return attrs;
}

/*
vi:ts=4:nowrap:expandtab
*/
