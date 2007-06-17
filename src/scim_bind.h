/**
 * @file scim_bind.h
 * @brief Binding adapters.
 * 
 * A binding adaptor is an object that allows you to convert between slot types.
 * Usually you wont use a BoundSlot directly but instead call the bind() factory
 * function (similiar to the slot() factory function) which will create an
 * appropriate bound slot for you, depending on the parameters passed.
 *
 * Most code of this file are came from Inti project.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 * Copyright (c) 2002 The Inti Development Team.
 * Copyright (c) 2000 Red Hat, Inc.
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
 * $Id: scim_bind.h,v 1.10 2005/01/10 08:30:52 suzhe Exp $
 */


#ifndef __SCIM_BOUND_SLOT_H
#define __SCIM_BOUND_SLOT_H

namespace scim {
/**
 * @addtogroup SignalSlot
 * The classes for signal/slot mechanism.
 * @{
 */

//! @name Bind functions returning a new BoundSlot.
//! @{

//! @class BoundSlot0_1 
//! @brief Converts a slot taking one argument into a slot taking no arguments.

template<typename R, typename P1>
class BoundSlot0_1 : public Slot0<R>
{
    Pointer < Slot1<R, P1> > original_slot;
    P1 p;

public:
    BoundSlot0_1(Slot1<R, P1> *slot, P1 p1) : original_slot(slot), p(p1) {}
    //!< Constructor.
    //!< @param slot - a pointer to a slot of type Slot1<R, P1>.
    //!< @param p1 - a bound argument of type P1

    virtual R call() const { return original_slot->call(p); }
    //!< Calls the original slot passing it the bound argument p as the last parameter.
};

//! Overloaded bind() factory function.
//! @param s - a slot of type Slot1<R, P1>.
//! @param p1 - a value of type P1.
//! @return a new slot that stores the value p1.
//!
//! <BR>When then returned slot is called it calls the original slot <EM>s</EM>, passing
//! it the arguments passed to it and the value <EM>p1</EM>, as the last parameter. If
//! the returned slot is connected to a signal it doesn't have to be unreferenced. The
//! signal it's connected to will unreference the slot when it is destroyed. Otherwise
//! the slot must be unreferenced by calling unref().

template<typename R, typename P1>
inline Slot0<R>*
bind(Slot1<R, P1> *s, P1 p1)
{
    return new BoundSlot0_1<R, P1>(s, p1);
}

//! @class BoundSlot1_2
//! @brief Converts a slot taking two arguments into a slot taking one argument.

template<typename R, typename P1, typename P2>
class BoundSlot1_2 : public Slot1<R, P1>
{
    Pointer < Slot2<R, P1, P2> > original_slot;
    P2 p;

public:
    BoundSlot1_2(Slot2<R, P1, P2> *slot, P2 p2) : original_slot(slot), p(p2) {}
    //!< Constructor.
    //!< @param slot - a pointer to a slot of type Slot1<R, P1, P2>.
    //!< @param p2 - a bound argument of type P2

    virtual R call(P1 p1) const { return original_slot->call(p1, p); }
    //!< Calls the original slot passing it argument p1 and the bound argument p as the last parameter.
};

//! Overloaded bind() factory function.
//! @param s - a slot of type Slot1<R, P1, P2>.
//! @param p2 - a value of type P2.
//! @return a new slot that stores the value p2.
//!
//! <BR>When then returned slot is called it calls the original slot <EM>s</EM>, passing
//! it the arguments passed to it and the value <EM>p2</EM>, as the last parameter. If
//! the returned slot is connected to a signal it doesn't have to be unreferenced. The
//! signal it's connected to will unreference the slot when it is destroyed. Otherwise
//! the slot must be unreferenced by calling unref().

template<typename R, typename P1, typename P2>
inline Slot1<R, P1>*
bind(Slot2<R, P1, P2> *s, P2 p2)
{
    return new BoundSlot1_2<R, P1, P2>(s, p2);
}

//! @class BoundSlot2_3
//! @brief Converts a slot taking three arguments into a slot taking two arguments.

template<typename R, typename P1, typename P2, typename P3>
class BoundSlot2_3 : public Slot2<R, P1, P2>
{
    Pointer < Slot3<R, P1, P2, P3> > original_slot;
    P3 p;

public:
    BoundSlot2_3(Slot3<R, P1, P2, P3> *slot, P3 p3) : original_slot(slot), p(p3) {}
    //!< Constructor.
    //!< @param slot - a pointer to a slot of type Slot1<R, P1, P2, P3>.
    //!< @param p3 - a bound argument of type P3

    virtual R call(P1 p1, P2 p2) const { return original_slot->call(p1, p2, p); }
    //!< Calls the original slot passing it arguments p1 and p2, and the bound argument p as the last parameter.
};
 
//! Overloaded bind() factory function.
//! @param s - a slot of type Slot1<R, P1, P2, P3>.
//! @param p3 - a value of type P3.
//! @return a new slot that stores the value p3.
//!
//! <BR>When then returned slot is called it calls the original slot <EM>s</EM>, passing
//! it the arguments passed to it and the value <EM>p3</EM>, as the last parameter. If
//! the returned slot is connected to a signal it doesn't have to be unreferenced. The
//! signal it's connected to will unreference the slot when it is destroyed. Otherwise
//! the slot must be unreferenced by calling unref().

template<typename R, typename P1, typename P2, typename P3>
inline Slot2<R, P1, P2>*
bind(Slot3<R, P1, P2, P3> *s, P3 p3)
{
    return new BoundSlot2_3<R, P1, P2, P3>(s, p3);
}

//! @class BoundSlot3_4
//! @brief Converts a slot taking four arguments into a slot taking three arguments.

template<typename R, typename P1, typename P2, typename P3, typename P4>
class BoundSlot3_4 : public Slot3<R, P1, P2, P3>
{
    Pointer < Slot4<R, P1, P2, P3, P4> > original_slot;
    P4 p;

public:
    BoundSlot3_4(Slot4<R, P1, P2, P3, P4> *slot, P4 p4) : original_slot(slot), p(p4) {}
    //!< Constructor.
    //!< @param slot - a pointer to a slot of type Slot1<R, P1, P2, P3, P4>.
    //!< @param p4 - a bound argument of type P4

    virtual R call(P1 p1, P2 p2, P3 p3) const { return original_slot->call(p1, p2, p3, p); }
    //!< Calls the original slot passing it arguments p1, p2 and p3, and the bound argument p as the last parameter.
};
 
//! Overloaded bind() factory function.
//! @param s - a slot of type Slot1<R, P1, P2, P3, P4>.
//! @param p4 - a value of type P4.
//! @return a new slot that stores the value p4.
//!
//! <BR>When then returned slot is called it calls the original slot <EM>s</EM>, passing
//! it the arguments passed to it and the value <EM>p4</EM>, as the last parameter. If
//! the returned slot is connected to a signal it doesn't have to be unreferenced. The
//! signal it's connected to will unreference the slot when it is destroyed. Otherwise
//! the slot must be unreferenced by calling unref().

template<typename R, typename P1, typename P2, typename P3, typename P4>
inline Slot3<R, P1, P2, P3>*
bind(Slot4<R, P1, P2, P3, P4> *s, P4 p4)
{
    return new BoundSlot3_4<R, P1, P2, P3, P4>(s, p4);
}

//! @class BoundSlot4_5
//! @brief Converts a slot taking five arguments into a slot taking four arguments.

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class BoundSlot4_5 : public Slot4<R, P1, P2, P3, P4>
{
    Pointer < Slot5<R, P1, P2, P3, P4, P5> > original_slot;
    P5 p;

public:
    BoundSlot4_5(Slot5<R, P1, P2, P3, P4, P5> *slot, P5 p5) : original_slot(slot), p(p5) {}
    //!< Constructor.
    //!< @param slot - a pointer to a slot of type Slot1<R, P1, P2, P3, P4, P5>.
    //!< @param p5 - a bound argument of type P5

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4) const { return original_slot->call(p1, p2, p3, p4, p); }
    //!< Calls the original slot passing it arguments p1, p2, p3 and p4, and the bound argument p as the last parameter.
};
 
//! Overloaded bind() factory function.
//! @param s - a slot of type Slot1<R, P1, P2, P3, P4, P5>.
//! @param p5 - a value of type P5.
//! @return a new slot that stores the value p5.
//!
//! <BR>When then returned slot is called it calls the original slot <EM>s</EM>, passing
//! it the arguments passed to it and the value <EM>p5</EM>, as the last parameter. If
//! the returned slot is connected to a signal it doesn't have to be unreferenced. The
//! signal it's connected to will unreference the slot when it is destroyed. Otherwise
//! the slot must be unreferenced by calling unref().

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
inline Slot4<R, P1, P2, P3, P4>*
bind(Slot5<R, P1, P2, P3, P4, P5> *s, P5 p5)
{
    return new BoundSlot4_5<R, P1, P2, P3, P4, P5>(s, p5);
}

//! @class BoundSlot5_6
//! @brief Converts a slot taking six arguments into a slot taking five arguments.

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class BoundSlot5_6 : public Slot5<R, P1, P2, P3, P4, P5>
{
    Pointer < Slot6<R, P1, P2, P3, P4, P5, P6> > original_slot;
    P6 p;

public:
    BoundSlot5_6(Slot6<R, P1, P2, P3, P4, P5, P6> *slot, P6 p6) : original_slot(slot), p(p6) {}
    //!< Constructor.
    //!< @param slot - a pointer to a slot of type Slot1<R, P1, P2, P3, P4, P5, P6>.
    //!< @param p6 - a bound argument of type P6

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const { return original_slot->call(p1, p2, p3, p4, p5, p); }
    //!< Calls the original slot passing it arguments p1, p2, p3, p4 and p5, and the bound argument p as the last parameter.
};
 
//! Overloaded bind() factory function.
//! @param s - a slot of type Slot1<R, P1, P2, P3, P4, P5, P6>.
//! @param p6 - a value of type P6.
//! @return a new slot that stores the value p6.
//!
//! <BR>When then returned slot is called it calls the original slot <EM>s</EM>, passing
//! it the arguments passed to it and the value <EM>p6</EM>, as the last parameter. If
//! the returned slot is connected to a signal it doesn't have to be unreferenced. The
//! signal it's connected to will unreference the slot when it is destroyed. Otherwise
//! the slot must be unreferenced by calling unref().

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
inline Slot5<R, P1, P2, P3, P4, P5>*
bind(Slot6<R, P1, P2, P3, P4, P5, P6> *s, P6 p6)
{
    return new BoundSlot5_6<R, P1, P2, P3, P4, P5, P6>(s, p6);
}

//! @}

/** @} */

} // namespace scim

#endif //__SCIM_BOUND_SLOT_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

