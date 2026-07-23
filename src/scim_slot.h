/** @file scim_slot.h
 * @brief C++ slot interface.
 *
 * Provides slot class templates. Slots are callable objects that can be used
 * to connect functions, class methods and function objects to scim::Signals.
 *
 * The typed slot is the variadic template SlotN<R, Args...>. The historical
 * fixed-arity names Slot0 .. Slot6 are kept as aliases for source
 * compatibility.
 *
 * Most code of this file came from the Inti project.
 */

/*
 * Smart Common Input Method
 *
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 * Copyright (c) 2002 The Inti Development Team.
 * Copyright (c) 2000 Red Hat, Inc.
 * Copyright 1999, Karl Einar Nelson
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
 * $Id: scim_slot.h,v 1.8 2005/01/10 08:30:54 suzhe Exp $
 */

#pragma once

namespace scim {

/**
 * @addtogroup SignalSlot
 * @{
 */

//! @name Slot functions returning a new slot
//! @{

//! @class Slot
//! @brief Base class for slots that can connect to scim::Signals.

class Slot : public ReferencedObject
{
    Slot(const Slot&);
    Slot& operator=(const Slot&);

protected:
    Slot();
    //!< Constructor.

    virtual ~Slot() = 0;
    //!< Destructor.
};

//! @class SlotN
//! @brief Base class template for slots passing Args... and returning R.
//!
//! The fixed-arity aliases Slot0<R>, Slot1<R,P1>, ... Slot6<R,...> below name
//! specialisations of this template.

template <typename R, typename... Args>
class SlotN : public Slot
{
protected:
    SlotN() {}
    //!< Constructor.

public:
    virtual R call(Args... args) const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()(Args... args) const { return call(args...); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlotN
//! @brief A slot template for static functions taking Args... and returning R.

template <typename R, typename... Args>
class FunctionSlotN : public SlotN<R, Args...>
{
    typedef R (*PF)(Args...);
    PF pf;

public:
    FunctionSlotN(PF function) : pf(function) {}
    //!< Construct a new function slot for a static function.

    virtual R call(Args... args) const { return (*pf)(args...); }
    //!< Calls the function connected to this slot.
};

//! Overloaded slot factory function for a static function.
//! @return a new slot passing Args... and returning R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R, typename... Args>
inline SlotN<R, Args...>*
slot(R (*function)(Args...))
{
    return new FunctionSlotN<R, Args...>(function);
}

//! @class MethodSlotN
//! @brief A slot template for methods of a class T taking Args... and returning R.

template <typename T, typename R, typename... Args>
class MethodSlotN : public SlotN<R, Args...>
{
    typedef R (T::*PMF)(Args...);
    PMF pmf;
    T *t;

public:
    MethodSlotN(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.

    virtual R call(Args... args) const { return (t->*pmf)(args...); }
    //!< Calls the class method connected to this slot.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(Args...).
//! @return a new slot passing Args... and returning R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename... Args>
inline SlotN<R, Args...>*
slot(T1* &object, R (T2::*function)(Args...))
{
    return new MethodSlotN<T2, R, Args...>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).

template <typename T1, typename T2, typename R, typename... Args>
inline SlotN<R, Args...>*
slot(T1* const &object, R (T2::*function)(Args...))
{
    return new MethodSlotN<T2, R, Args...>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.

template <typename T1, typename T2, typename R, typename... Args>
inline SlotN<R, Args...>*
slot(T1& object, R (T2::*function)(Args...))
{
    return new MethodSlotN<T2, R, Args...>(&object, function);
}

//! @class SignalSlotN
//! @brief A slot template that forwards to another signal's callback.

template <typename T, typename R, typename... Args>
class SignalSlotN : public SlotN<R, Args...>
{
    typedef R (*PF)(void*, Args...);
    PF pf;
    T *t;

public:
    SignalSlotN(T *signal, PF function) : pf(function), t(signal) {}

    virtual R call(Args... args) const { return (*pf)(t, args...); }
};

//
// Fixed-arity aliases kept for source compatibility. New code can use
// SlotN<R, Args...> directly.
//

template <typename R>
using Slot0 = SlotN<R>;

template <typename R, typename P1>
using Slot1 = SlotN<R, P1>;

template <typename R, typename P1, typename P2>
using Slot2 = SlotN<R, P1, P2>;

template <typename R, typename P1, typename P2, typename P3>
using Slot3 = SlotN<R, P1, P2, P3>;

template <typename R, typename P1, typename P2, typename P3, typename P4>
using Slot4 = SlotN<R, P1, P2, P3, P4>;

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
using Slot5 = SlotN<R, P1, P2, P3, P4, P5>;

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
using Slot6 = SlotN<R, P1, P2, P3, P4, P5, P6>;

//! @}

/** @} */

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
