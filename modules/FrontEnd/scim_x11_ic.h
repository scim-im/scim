/** @file scim_x11_ic.h
 * definition of X11IC related classes.
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
 * $Id: scim_x11_ic.h,v 1.10 2005/06/26 16:35:12 suzhe Exp $
 */

#if !defined (__SCIM_X11_IC_H)
#define __SCIM_X11_IC_H

#include "scim_stl_map.h"

using namespace scim;

struct X11PreeditAttributes
{
    XRectangle area;        /* area */
    XRectangle area_needed;    /* area needed */
    XPoint spot_location;    /* spot location */
    Colormap cmap;            /* colormap */
    CARD32 foreground;        /* foreground */
    CARD32 background;        /* background */
    Pixmap bg_pixmap;        /* background pixmap */
    String base_font;        /* base font of fontset */
    CARD32 line_space;        /* line spacing */
    Cursor cursor;            /* cursor */
};

struct X11StatusAttributes
{
    XRectangle area;        /* area */
    XRectangle area_needed;    /* area needed */
    Colormap cmap;            /* colormap */
    CARD32 foreground;        /* foreground */
    CARD32 background;        /* background */
    Pixmap bg_pixmap;        /* background pixmap */
    String base_font;        /* base font of fontset */
    CARD32 line_space;        /* line spacing */
    Cursor cursor;            /* cursor */
};

struct X11IC
{
    int    siid;                    /* server instance id */
    CARD16 icid;                    /* ic id */
    CARD16 connect_id;              /* connect id */
    INT32  input_style;             /* input style */
    Window client_win;              /* client window */
    Window focus_win;               /* focus window */
    String encoding;                /* connection encoding */
    String locale;                  /* connection locale */
    X11PreeditAttributes pre_attr;  /* preedit attributes */
    X11StatusAttributes  sts_attr;  /* status attributes */

    bool shared_siid;
    bool xims_on;
    bool onspot_preedit_started; 
    int  onspot_preedit_length;     /* preedit length of onspot mode */
    int  onspot_caret;              /* caret position of onspot mode */

    X11IC *next;
};

#define SCIM_X11_IC_INPUT_STYLE         (1<<0)
#define SCIM_X11_IC_CLIENT_WINDOW       (1<<1)
#define SCIM_X11_IC_FOCUS_WINDOW        (1<<2)
#define SCIM_X11_IC_ENCODING            (1<<3)
#define SCIM_X11_IC_PRE_AREA            (1<<4)
#define SCIM_X11_IC_PRE_AREA_NEEDED     (1<<5)
#define SCIM_X11_IC_PRE_SPOT_LOCATION   (1<<6)
#define SCIM_X11_IC_PRE_COLORMAP        (1<<7)
#define SCIM_X11_IC_PRE_FOREGROUND      (1<<8)
#define SCIM_X11_IC_PRE_BACKGROUND      (1<<9)
#define SCIM_X11_IC_PRE_BG_PIXMAP       (1<<10)
#define SCIM_X11_IC_PRE_FONTSET         (1<<11)
#define SCIM_X11_IC_PRE_LINE_SPACE      (1<<12)
#define SCIM_X11_IC_PRE_CURSOR          (1<<13)
#define SCIM_X11_IC_STS_AREA            (1<<14)
#define SCIM_X11_IC_STS_AREA_NEEDED     (1<<15)
#define SCIM_X11_IC_STS_COLORMAP        (1<<16)
#define SCIM_X11_IC_STS_FOREGROUND      (1<<17)
#define SCIM_X11_IC_STS_BACKGROUND      (1<<18)
#define SCIM_X11_IC_STS_BG_PIXMAP       (1<<19)
#define SCIM_X11_IC_STS_FONTSET         (1<<20)
#define SCIM_X11_IC_STS_LINE_SPACE      (1<<21)
#define SCIM_X11_IC_STS_CURSOR          (1<<22)
#define SCIM_X11_IC_FILTER_EVENTS       (1<<23)

class X11ICManager
{
#if SCIM_USE_STL_EXT_HASH_MAP
    typedef __gnu_cxx::hash_map <int, String, __gnu_cxx::hash <int> > ConnectionLocaleMap;
#elif SCIM_USE_STL_HASH_MAP
    typedef std::hash_map <int, String, std::hash <int> >             ConnectionLocaleMap;
#else
    typedef std::map <int, String>                                    ConnectionLocaleMap;
#endif

    X11IC *m_ic_list;
    X11IC *m_free_list;

    ConnectionLocaleMap m_connect_locales;

public:
    X11ICManager ();
    ~X11ICManager ();

private:
    /**
     * create an empty scimX11IC struct.
     */
    X11IC * new_ic ();

    /**
     * store the attributes in call_data to rec.
     * @return a bitsets indicates which attributes are set.
     */
    uint32 store_ic_values (X11IC *rec, IMChangeICStruct *call_data);

    /**
     * delete a X11IC struct according to its icid.
     */
    void delete_ic (CARD16 icid);

public:
    void new_connection (IMOpenStruct *call_data);
    void delete_connection (IMCloseStruct *call_data);

    String get_connection_locale (CARD16 connect_id);

    /**
     * Create a new X11IC struct and set its attributes.
     * @return a bitsets indicates which attributes are set.
     */
    uint32 create_ic (IMChangeICStruct *call_data, int siid);

    X11IC * find_ic (CARD16 icid);
    X11IC * find_ic_by_siid (int siid);

    void destroy_ic (IMDestroyICStruct *call_data);

    /**
     * load the attributes of ic into call_data
     * @return a bitsets indicates which attributes are loaded.
     */
    uint32 get_ic_values (IMChangeICStruct *call_data);
    
    /**
     * store the attributes in call_data to rec.
     * @return a bitsets indicates which attributes are set.
     */
    uint32 set_ic_values (IMChangeICStruct *call_data);
};

#endif // _SCIM_X11_IC_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
