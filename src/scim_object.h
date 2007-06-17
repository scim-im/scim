/** @file scim_object.h
 *  @brief Reference counted base class interface.
 *
 * Provides a reference counted base class
 * for dynamic objects handled the scim smart pointer.
 *
 * Most code of this file are came from Inti project.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 * Copyright (c) 2002 The Inti Development Team.
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
 * $Id: scim_object.h,v 1.9 2005/01/10 08:30:54 suzhe Exp $
 */

#ifndef __SCIM_OBJECT_H
#define __SCIM_OBJECT_H

namespace scim {

/**
 * @addtogroup Accessories
 * @{
 */

/**
 * @class ReferencedObject 
 * @brief Reference counted base class.
 * 
 * ReferencedObject is a reference counting base class.
 * it has an integer reference counter so that dynamic objects
 * can have their memory allocation handled by the scim 
 * smart pointer: Pointer<>. This keeps the memory management
 * in scim consistent across all classes.
 * If you derive a class from ReferencedObject and allocate it
 * on the heap, you free the memory and destroy the object by
 * calling unref(), not delete.
 */
class ReferencedObject
{
    template<typename T> friend class Pointer;

    ReferencedObject(const ReferencedObject&);
    ReferencedObject& operator=(const ReferencedObject&);

    bool m_referenced;
    int  m_ref_count;

protected:
    ReferencedObject();
    //!< Constructor.

    virtual ~ReferencedObject() = 0;
    //!< Destructor.

    void set_referenced(bool reference);
    //!< Set the internal referenced flag.
    //!< @param reference - <EM>true</EM> if the initial reference count must be removed by owner.
    //!<
    //!< <BR>Called by derived classes to set the referenced flag. A object sets this flag
    //!< to true , indicating that it owns the initial reference count and unref() must be called.
public:
    bool is_referenced() const;
    //!< The referenced flag setting.
    //!< @return <EM>true</EM> if unref() must be explicitly called on this object.

    void ref();
    //!< Increase an object's reference count by one.

    void unref();
    //!< Decrease an object's reference count by one.
    //!< When the reference count becomes zero delete is called. Remember, with ReferencedObject
    //!< you must call unref() on dynmaically allocated objects, not delete.
};

/** @} */

} // namespace scim

#endif //__SCIM_OBJECT_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
