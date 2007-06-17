/** @file scim_slot.h
 * @brief C++ slot interface.
 * 
 * Provides a set of slot class templates. Slots are callable objects that
 * can be used to connect functions, class methods and function objects to
 * scim::Signals. 
 *
 * Most code of this file are came from Inti project.
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

#ifndef __SCIM_SLOT_H
#define __SCIM_SLOT_H

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

//! @class Slot0 
//! @brief Base class template for slots passing no arguments and returning a value of type R.

template <typename R>
class Slot0 : public Slot
{
protected:
    Slot0() {}
    //!< Constructor.

public:
    virtual R call() const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()() const { return call(); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlot0 
//! @brief A slot template for static functions taking no arguments and returning a value of type R.

template <typename R>
class FunctionSlot0 : public Slot0<R>
{
    typedef R (*PF)();
    PF pf;
    
public:
    FunctionSlot0(PF function) : pf(function) {} 
    //!< Construct a new function slot for a static function.
    //!< @param function - static function with the signature R (*PF)().

    virtual R call() const { return (*pf)(); }
    //!< Calls the function connected to this slot.
    //!< @return a value of type R returned by the called function.
};

//! Overloaded slot factory function.
//! @param function - a static function with the signature R (*function)().
//! @return a new slot passing no arguments and returning a value of type R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R>
inline Slot0<R>*
slot(R (*function)())
{
    return new FunctionSlot0<R>(function);
}

//! @class MethodSlot0 
//! @brief A slot template for methods in a class of type T taking no arguments and returning a value of type R.

template <typename T, typename R>
class MethodSlot0 : public Slot0<R>
{
    typedef R (T::*PMF)();
    PMF pmf;
    T *t;
    
public:
    MethodSlot0(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.
    //!< @param object - a pointer to an object of type T.
    //!< @param function - a class method with the signature R (T::*PMF)().

    virtual R call() const { return (t->*pmf)(); }
    //!< Calls the class method connected to this slot.
    //!< @return a value of type R returned by the called method.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)().
//! @return a new slot passing no arguments and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R>
inline Slot0<R>*
slot(T1* &object, R (T2::*function)())
{
    return new MethodSlot0<T2, R>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).
//! @param function - a class method with the signature R (T2::*function)().
//! @return a new slot passing no arguments and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R>
inline Slot0<R>*
slot(T1* const &object, R (T2::*function)())
{
    return new MethodSlot0<T2, R>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)().
//! @return a new slot passing no arguments and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R>
inline Slot0<R>*
slot(T1& object, R (T2::*function)())
{
    return new MethodSlot0<T2, R>(&object, function);
}

/*  SignalSlot0
 */

template <typename T, typename R>
class SignalSlot0 : public Slot0<R>
{
    typedef R (*PF)(void*);
    PF pf;
    T *t;
    
public:
    SignalSlot0(T *signal, PF function) : pf(function), t(signal) {} 

    virtual R call() const { return (*pf)(t); }
};

//! @class Slot1 
//! @brief Base class template for slots passing one argument of type P1 and returning a value of type R.

template <typename R, typename P1>
class Slot1 : public Slot
{
protected:
    Slot1() {}
    //!< Constructor.

public:
    virtual R call(P1 p1) const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()(P1 p1) const { return call(p1); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlot1 
//! @brief A slot template for static functions taking one argument of type P1
//! and returning a value of type R.

template <typename R, typename P1>
class FunctionSlot1 : public Slot1<R, P1>
{
    typedef R (*PF)(P1);
    PF pf;
    
public:
    FunctionSlot1(PF function) : pf(function) {} 
    //!< Construct a new function slot for a static function.
    //!< @param function - static function with the signature R (*PF)(P1).

    virtual R call(P1 p1) const { return (*pf)(p1); }
    //!< Calls the function connected to this slot passing it argument p1.
    //!< @return a value of type R returned by the called function.
};

//! Overloaded slot factory function.
//! @param function - a static function with the signature R (*function)(P1).
//! @return a new slot passing one argument of type P1 and returning a value of type R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R, typename P1>
inline Slot1<R, P1>*
slot(R (*function)(P1))
{
    return new FunctionSlot1<R, P1>(function);
}

//! @class MethodSlot1
//! @brief A slot template for methods in a class of type T taking one argument of type P1
//! and returning a value of type R.

template <typename T, typename R, typename P1>
class MethodSlot1 : public Slot1<R, P1>
{
    typedef R (T::*PMF)(P1);
    PMF pmf;
    T *t;
    
public:
    MethodSlot1(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.
    //!< @param object - a pointer to an object of type T.
    //!< @param function - a class method with the signature R (T::*PMF)(P1).

    virtual R call(P1 p1) const { return (t->*pmf)(p1); }
    //!< Calls the class method connected to this slot passing it argument p1.
    //!< @return a value of type R returned by the called method.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1).
//! @return a new slot passing one argument of type P1 and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1>
inline Slot1<R, P1>*
slot(T1* &object, R (T2::*function)(P1))
{
    return new MethodSlot1<T2, R, P1>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).
//! @param function - a class method with the signature R (T2::*function)(P1).
//! @return a new slot passing one argument of type P1 and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1>
inline Slot1<R, P1>*
slot(T1* const &object, R (T2::*function)(P1))
{
    return new MethodSlot1<T2, R, P1>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1).
//! @return a new slot passing one argument of type P1 and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1>
inline Slot1<R, P1>*
slot(T1& object, R (T2::*function)(P1))
{
    return new MethodSlot1<T2, R, P1>(&object, function);
}

/*  SignalSlot1
 */

template <typename T, typename R, typename P1>
class SignalSlot1 : public Slot1<R, P1>
{
    typedef R (*PF)(void*, P1);
    PF pf;
    T *t;
    
public:
    SignalSlot1(T *signal, PF function) : pf(function), t(signal) {} 

    virtual R call(P1 p1) const { return (*pf)(t, p1); }
};

//! @class Slot2 
//! @brief Base class template for slots passing two arguments of type P1 and P2,
//! and returning a value of type R.

template <typename R, typename P1, typename P2>
class Slot2 : public Slot
{
protected:
    Slot2() {}
    //!< Constructor.
    
public:
    virtual R call(P1 p1, P2 p2) const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()(P1 p1, P2 p2) const { return call(p1, p2); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlot2 
//! @brief A slot template for static functions taking two arguments of type P1 and P2,
//! and returning a value of type R.

template <typename R, typename P1, typename P2>
class FunctionSlot2 : public Slot2<R, P1, P2>
{
    typedef R (*PF)(P1, P2);
    PF pf;
    
public:
    FunctionSlot2(PF function) : pf(function) {} 
    //!< Construct a new function slot for a static function.
    //!< @param function - static function with the signature R (*PF)(P1, P2).

    virtual R call(P1 p1, P2 p2) const { return (*pf)(p1, p2); }
    //!< Calls the function connected to this slot passing it arguments p1 and p2.
    //!< @return a value of type R returned by the called function.
};

//! Overloaded slot factory function.
//! @param function - a static function with the signature R (*function)(P1, P2).
//! @return a new slot passing two arguments of type P1 and P2, and returning a value of type R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R, typename P1, typename P2>
inline Slot2<R, P1, P2>*
slot(R (*function)(P1, P2))
{
    return new FunctionSlot2<R, P1, P2>(function);
}

//! @class MethodSlot2
//! @brief A slot template for methods in a class of type T taking two arguments of type P1 and P2,
//! and returning a value of type R.

template <typename T, typename R, typename P1, typename P2>
class MethodSlot2 : public Slot2<R, P1, P2>
{
    typedef R (T::*PMF)(P1, P2);
    PMF pmf;
    T *t;
    
public:
    MethodSlot2(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.
    //!< @param object - a pointer to an object of type T.
    //!< @param function - a class method with the signature R (T::*PMF)(P1, P2).

    virtual R call(P1 p1, P2 p2) const { return (t->*pmf)(p1, p2); }
    //!< Calls the class method connected to this slot passing it arguments p1 and p2.
    //!< @return a value of type R returned by the called method.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2).
//! @return a new slot passing two arguments of type P1 and P2, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2>
inline Slot2<R, P1, P2>*
slot(T1* &object, R (T2::*function)(P1, P2))
{
    return new MethodSlot2<T2, R, P1, P2>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).
//! @param function - a class method with the signature R (T2::*function)(P1, P2).
//! @return a new slot passing two arguments of type P1 and P2, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2>
inline Slot2<R, P1, P2>*
slot(T1* const &object, R (T2::*function)(P1, P2))
{
    return new MethodSlot2<T2, R, P1, P2>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2).
//! @return a new slot passing two arguments of type P1 and P2, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2>
inline Slot2<R, P1, P2>*
slot(T1& object, R (T2::*function)(P1, P2))
{
    return new MethodSlot2<T2, R, P1, P2>(&object, function);
}

/*  SignalSlot2
 */

template <typename T, typename R, typename P1, typename P2>
class SignalSlot2 : public Slot2<R, P1, P2>
{
    typedef R (*PF)(void*, P1, P2);
    PF pf;
    T *t;
    
public:
    SignalSlot2(T *signal, PF function) : pf(function), t(signal) {} 

    virtual R call(P1 p1, P2 p2) const { return (*pf)(t, p1, p2); }
};

//! @class Slot3 
//! @brief Base class template for slots passing three arguments of type P1, P2 and P3,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3>
class Slot3 : public Slot
{
protected:
    Slot3() {}
    //!< Constructor.

public:
    virtual R call(P1 p1, P2 p2, P3 p3) const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()(P1 p1, P2 p2, P3 p3) const { return call(p1, p2, p3); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlot3 
//! @brief A slot template for static functions taking three arguments of type P1, P2 and P3,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3>
class FunctionSlot3 : public Slot3<R, P1, P2, P3>
{
    typedef R (*PF)(P1, P2, P3);
    PF pf;
    
public:
    FunctionSlot3(PF function) : pf(function) {} 
    //!< Construct a new function slot for a static function.
    //!< @param function - static function with the signature R (*PF)(P1, P2, P3).

    virtual R call(P1 p1, P2 p2, P3 p3) const { return (*pf)(p1, p2, p3); }
    //!< Calls the function connected to this slot passing it arguments p1, p2 and p3.
    //!< @return a value of type R returned by the called function.
};

//! Overloaded slot factory function.
//! @param function - a static function with the signature R (*function)(P1, P2, P3).
//! @return a new slot passing three arguments of type P1, P2 and P3, and returning a value of type R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R, typename P1, typename P2, typename P3>
inline Slot3<R, P1, P2, P3>*
slot(R (*function)(P1, P2, P3))
{
    return new FunctionSlot3<R, P1, P2, P3>(function);
}

//! @class MethodSlot3
//! @brief A slot template for methods in a class of type T taking three arguments of type P1, P2 and P3,
//! and returning a value of type R.

template <typename T, typename R, typename P1, typename P2, typename P3>
class MethodSlot3 : public Slot3<R, P1, P2, P3>
{
    typedef R (T::*PMF)(P1, P2, P3);
    PMF pmf;
    T *t;
    
public:
    MethodSlot3(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.
    //!< @param object - a pointer to an object of type T.
    //!< @param function - a class method with the signature R (T::*PMF)(P1, P2, P3).

    virtual R call(P1 p1, P2 p2, P3 p3) const { return (t->*pmf)(p1, p2, p3); }
    //!< Calls the class method connected to this slot passing it arguments p1, p2 and p3.
    //!< @return a value of type R returned by the called method.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3).
//! @return a new slot passing three arguments of type P1, P2 and P3, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3>
inline Slot3<R, P1, P2, P3>*
slot(T1* &object, R (T2::*function)(P1, P2, P3))
{
    return new MethodSlot3<T2, R, P1, P2, P3>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3).
//! @return a new slot passing three arguments of type P1, P2 and P3, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3>
inline Slot3<R, P1, P2, P3>*
slot(T1* const &object, R (T2::*function)(P1, P2, P3))
{
    return new MethodSlot3<T2, R, P1, P2, P3>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3).
//! @return a new slot passing three arguments of type P1, P2 and P3, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3>
inline Slot3<R, P1, P2, P3>*
slot(T1& object, R (T2::*function)(P1, P2, P3))
{
    return new MethodSlot3<T2, R, P1, P2, P3>(&object, function);
}

/*  SignalSlot3
 */

template <typename T, typename R, typename P1, typename P2, typename P3>
class SignalSlot3 : public Slot3<R, P1, P2, P3>
{
    typedef R (*PF)(void*, P1, P2, P3);
    PF pf;
    T *t;
    
public:
    SignalSlot3(T *signal, PF function) : pf(function), t(signal) {} 

    virtual R call(P1 p1, P2 p2, P3 p3) const { return (*pf)(t, p1, p2, p3); }
};

//! @class Slot4
//! @brief Base class template for slots passing four arguments of type P1, P2, P3 and P4,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3, typename P4>
class Slot4 : public Slot
{
protected:
    Slot4() {}
    //!< Constructor.

public:
    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4) const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4) const { return call(p1, p2, p3, p4); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlot4 
//! @brief A slot template for static functions taking four arguments of type P1, P2, P3 and P4,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3, typename P4>
class FunctionSlot4 : public Slot4<R, P1, P2, P3, P4>
{
    typedef R (*PF)(P1, P2, P3, P4);
    PF pf;
    
public:
    FunctionSlot4(PF function) : pf(function) {} 
    //!< Construct a new function slot for a static function.
    //!< @param function - static function with the signature R (*PF)(P1, P2, P3, P4).

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4) const { return (*pf)(p1, p2, p3, p4); }
    //!< Calls the function connected to this slot passing it arguments p1, p2, p3 and p4.
    //!< @return a value of type R returned by the called function.
};

//! Overloaded slot factory function.
//! @param function - a static function with the signature R (*function)(P1, P2, P3, P4).
//! @return a new slot passing four arguments of type P1, P2, P3 and P4, and returning a value of type R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R, typename P1, typename P2, typename P3, typename P4>
inline Slot4<R, P1, P2, P3, P4>*
slot(R (*function)(P1, P2, P3, P4))
{
    return new FunctionSlot4<R, P1, P2, P3, P4>(function);
}

//! @class MethodSlot4 
//! @brief A slot template for methods in a class of type T taking four arguments of type P1, P2, P3 and P4,
//! and returning a value of type R.

template <typename T, typename R, typename P1, typename P2, typename P3, typename P4>
class MethodSlot4 : public Slot4<R, P1, P2, P3, P4>
{
    typedef R (T::*PMF)(P1, P2, P3, P4);
    PMF pmf;
    T *t;
    
public:
    MethodSlot4(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.
    //!< @param object - a pointer to an object of type T.
    //!< @param function - a class method with the signature R (T::*PMF)(P1, P2, P3, P4).

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4) const { return (t->*pmf)(p1, p2, p3, p4); }
    //!< Calls the class method connected to this slot passing it arguments p1, p2, p3 and p4.
    //!< @return a value of type R returned by the called method.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4).
//! @return a new slot passing four arguments of type P1, P2, P3 and P4, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4>
inline Slot4<R, P1, P2, P3, P4>*
slot(T1* &object, R (T2::*function)(P1, P2, P3, P4))
{
    return new MethodSlot4<T2, R, P1, P2, P3, P4>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4).
//! @return a new slot passing four arguments of type P1, P2, P3 and P4, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4>
inline Slot4<R, P1, P2, P3, P4>*
slot(T1* const &object, R (T2::*function)(P1, P2, P3, P4))
{
    return new MethodSlot4<T2, R, P1, P2, P3, P4>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4).
//! @return a new slot passing four arguments of type P1, P2, P3 and P4, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4>
inline Slot4<R, P1, P2, P3, P4>*
slot(T1& object, R (T2::*function)(P1, P2, P3, P4))
{
    return new MethodSlot4<T2, R, P1, P2, P3, P4>(&object, function);
}

/*  SignalSlot4
 */

template <typename T, typename R, typename P1, typename P2, typename P3, typename P4>
class SignalSlot4 : public Slot4<R, P1, P2, P3, P4>
{
    typedef R (*PF)(void*, P1, P2, P3, P4);
    PF pf;
    T *t;
    
public:
    SignalSlot4(T *signal, PF function) : pf(function), t(signal) {} 

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4) const { return (*pf)(t, p1, p2, p3, p4); }
};

//! @class Slot5 
//! @brief Base class template for slots passing five arguments of type P1, P2, P3, P4 and P5,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class Slot5 : public Slot
{
protected:
    Slot5() {}

public:
    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const { return call(p1, p2, p3, p4, p5); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlot5
//! @brief A slot template for static functions taking five arguments of type P1, P2, P3, P4 and P5,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class FunctionSlot5 : public Slot5<R, P1, P2, P3, P4, P5>
{
    typedef R (*PF)(P1, P2, P3, P4, P5);
    PF pf;
    
public:
    FunctionSlot5(PF function) : pf(function) {} 
    //!< Construct a new function slot for a static function.
    //!< @param function - static function with the signature R (*PF)(P1, P2, P3, P4, P5).

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const { return (*pf)(p1, p2, p3, p4, p5); }
    //!< Calls the function connected to this slot passing it arguments p1, p2, p3, p4 and p5.
    //!< @return a value of type R returned by the called function.
};

//! Overloaded slot factory function.
//! @param function - a static function with the signature R (*function)(P1, P2, P3, P4, P5).
//! @return a new slot passing five arguments of type P1, P2, P3, P4 and P5, and returning a value of type R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
inline Slot5<R, P1, P2, P3, P4, P5>*
slot(R (*function)(P1, P2, P3, P4, P5))
{
    return new FunctionSlot5<R, P1, P2, P3, P4, P5>(function);
}

//! @class MethodSlot5 
//! @brief A slot template for methods in a class of type T taking five arguments of type P1, P2, P3, P4 and P5,
//! and returning a value of type R.

template <typename T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class MethodSlot5 : public Slot5<R, P1, P2, P3, P4, P5>
{
    typedef R (T::*PMF)(P1, P2, P3, P4, P5);
    PMF pmf;
    T *t;
    
public:
    MethodSlot5(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.
    //!< @param object - a pointer to an object of type T.
    //!< @param function - a class method with the signature R (T::*PMF)(P1, P2, P3, P4, P5).

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const { return (t->*pmf)(p1, p2, p3, p4, p5); }
    //!< Calls the class method connected to this slot passing it arguments p1, p2, p3, p4 and p5.
    //!< @return a value of type R returned by the called method.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4, P5).
//! @return a new slot passing five arguments of type P1, P2, P3, P4 and P5, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
inline Slot5<R, P1, P2, P3, P4, P5>*
slot(T1* &object, R (T2::*function)(P1, P2, P3, P4, P5))
{
    return new MethodSlot5<T2, R, P1, P2, P3, P4, P5>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4, P5).
//! @return a new slot passing five arguments of type P1, P2, P3, P4 and P5, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
inline Slot5<R, P1, P2, P3, P4, P5>*
slot(T1* const &object, R (T2::*function)(P1, P2, P3, P4, P5))
{
    return new MethodSlot5<T2, R, P1, P2, P3, P4, P5>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4, P5).
//! @return a new slot passing five arguments of type P1, P2, P3, P4 and P5, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
inline Slot5<R, P1, P2, P3, P4, P5>*
slot(T1& object, R (T2::*function)(P1, P2, P3, P4, P5))
{
    return new MethodSlot5<T2, R, P1, P2, P3, P4, P5>(&object, function);
}

/*  SignalSlot5
 */

template <typename T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
class SignalSlot5 : public Slot5<R, P1, P2, P3, P4, P5>
{
    typedef R (*PF)(void*, P1, P2, P3, P4, P5);
    PF pf;
    T *t;
    
public:
    SignalSlot5(T *signal, PF function) : pf(function), t(signal) {} 

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const { return (*pf)(t, p1, p2, p3, p4, p5); }
};

//! @class Slot6 
//! @brief Base class template for slots passing six arguments of type P1, P2, P3, P4, P5 and P6,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class Slot6 : public Slot
{
protected:
    Slot6() {}
    //!< Constructor.

public:
    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const = 0;
    //!< Calls the signal handler connected to this slot.

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const { return call(p1, p2, p3, p4, p5, p6); }
    //!< Function operator; Calls call().
};

//! @class FunctionSlot6
//! @brief A slot template for static functions taking six arguments of type P1, P2, P3, P4, P5 and P6,
//! and returning a value of type R.

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class FunctionSlot6 : public Slot6<R, P1, P2, P3, P4, P5, P6>
{
    typedef R (*PF)(P1, P2, P3, P4, P5, P6);
    PF pf;
    
public:
    FunctionSlot6(PF function) : pf(function) {} 
    //!< Construct a new function slot for a static function.
    //!< @param function - static function with the signature R (*PF)(P1, P2, P3, P4, P5).

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const { return (*pf)(p1, p2, p3, p4, p5, p6); }
    //!< Calls the function connected to this slot passing it arguments p1, p2, p3, p4, p5 and p6.
    //!< @return a value of type R returned by the called function.
};

//! Overloaded slot factory function.
//! @param function - a static function with the signature R (*function)(P1, P2, P3, P4, P5, P6).
//! @return a new slot passing six arguments of type P1, P2, P3, P4, P5 and P6, and returning a value of type R.
//!
//! <BR>If the returned slot is connected to a signal it doesn't have to be
//! unreferenced. The signal it's connected to will unreference the slot when
//! it is destroyed. Otherwise the slot must be unreferenced by calling unref().

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
inline Slot6<R, P1, P2, P3, P4, P5, P6>*
slot(R (*function)(P1, P2, P3, P4, P5, P6))
{
    return new FunctionSlot6<R, P1, P2, P3, P4, P5, P6>(function);
}

//! @class MethodSlot6
//! @brief A slot template for methods in a class of type T taking six arguments of type P1, P2, P3, P4, P5 and P6,
//! and returning a value of type R.

template <typename T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class MethodSlot6 : public Slot6<R, P1, P2, P3, P4, P5, P6>
{
    typedef R (T::*PMF)(P1, P2, P3, P4, P5, P6);
    PMF pmf;
    T *t;
    
public:
    MethodSlot6(T *object, PMF function) : pmf(function), t(object) {}
    //!< Construct a new method slot for a class member function.
    //!< @param object - a pointer to an object of type T.
    //!< @param function - a class method with the signature R (T::*PMF)(P1, P2, P3, P4, P5, P6).

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const { return (t->*pmf)(p1, p2, p3, p4, p5, p6); }
    //!< Calls the class method connected to this slot passing it arguments p1, p2, p3, p4, p5 and p6.
    //!< @return a value of type R returned by the called method.
};

//! Overloaded slot factory function.
//! @param object - a reference to a pointer to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4, P5, P6).
//! @return a new slot passing six arguments of type P1, P2, P3, P4, P5 and P6, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
inline Slot6<R, P1, P2, P3, P4, P5, P6>*
slot(T1* &object, R (T2::*function)(P1, P2, P3, P4, P5, P6))
{
    return new MethodSlot6<T2, R, P1, P2, P3, P4, P5, P6>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to a const pointer to an object of type T1 (e.g. this).
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4, P5, P6).
//! @return a new slot passing six arguments of type P1, P2, P3, P4, P5 and P6, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
inline Slot6<R, P1, P2, P3, P4, P5, P6>*
slot(T1* const &object, R (T2::*function)(P1, P2, P3, P4, P5, P6))
{
    return new MethodSlot6<T2, R, P1, P2, P3, P4, P5, P6>(object, function);
}

//! Overloaded slot factory function.
//! @param object - a reference to an object of type T1.
//! @param function - a class method with the signature R (T2::*function)(P1, P2, P3, P4, P5, P6).
//! @return a new slot passing six arguments of type P1, P2, P3, P4, P5 and P6, and returning a value of type R.
//!
//! <BR>T1 can be the same object type as T2. If the returned slot is connected
//! to a signal it doesn't have to be unreferenced. The signal it's connected to
//! will unreference the slot when it is destroyed. Otherwise the slot must be
//! unreferenced by calling unref().

template <typename T1, typename T2, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
inline Slot6<R, P1, P2, P3, P4, P5, P6>*
slot(T1& object, R (T2::*function)(P1, P2, P3, P4, P5, P6))
{
    return new MethodSlot6<T2, R, P1, P2, P3, P4, P5, P6>(&object, function);
}

/*  SignalSlot6
 */

template <typename T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
class SignalSlot6 : public Slot6<R, P1, P2, P3, P4, P5, P6>
{
    typedef R (*PF)(void*, P1, P2, P3, P4, P5, P6);
    PF pf;
    T *t;
    
public:
    SignalSlot6(T *signal, PF function) : pf(function), t(signal) {} 

    virtual R call(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const { return (*pf)(t, p1, p2, p3, p4, p5, p6); }
};

//! @}

/** @} */

} // namespace scim

#endif //__SCIM_SLOT_H

/*
vi:ts=4:nowrap:ai:expandtab
*/
