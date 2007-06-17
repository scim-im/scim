/** @file scim_rawcode_imengine.h
 * definition of RawCode related classes.
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
 * $Id: scim_test_imengine.h,v 1.2 2005/02/02 15:25:34 suzhe Exp $
 */

#if !defined (__SCIM_TEST_SERVER_H)
#define __SCIM_TEST_SERVER_H

using namespace scim;

class TestFactory : public IMEngineFactoryBase
{
    friend class TestInstance;

public:
    TestFactory ();
    virtual ~TestFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;
    virtual String      get_language () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding, int id = -1);
};

class TestInstance : public IMEngineInstanceBase
{
    bool m_helper_started;
    bool m_focused;
    bool m_work;

    Transaction m_trans;

public:
    TestInstance (TestFactory *factory,
                     const String& encoding,
                     int id = -1);
    virtual ~TestInstance ();

    virtual bool process_key_event (const KeyEvent& key);
    virtual void process_helper_event (const String &helper_uuid, const Transaction &trans);
    virtual void reset ();
    virtual void focus_in ();
    virtual void focus_out ();
};

#endif
/*
vi:ts=4:nowrap:ai:expandtab
*/

