/**
 * @file scim_exception.h
 * @brief Defines the scim::Exception class.
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
 * $Id: scim_exception.h,v 1.10 2005/01/10 08:30:53 suzhe Exp $
 */

#ifndef __SCIM_EXCEPTION_H
#define __SCIM_EXCEPTION_H

namespace scim {
/**
 * @addtogroup Accessories
 * @{
 */

/**
 * @brief A base class of all other exception classes.
 *
 * All other exception classes in namespace scim
 * should be derived from this class.
 */
class Exception: public std::exception
{
    String m_what;
public:
    Exception (const String& what_arg) : m_what (what_arg) { }
    ~Exception () throw () {}
    virtual const char* what () const throw () { return m_what.c_str (); }
};

/** @} */

} // namespace scim

#endif //__SCIM_EXCEPTION_H
/*
vi:ts=4:nowrap:ai:expandtab
*/
