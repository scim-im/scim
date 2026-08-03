/**
 * @file scim_signals.h
 * @brief C++ signal interface.
 *
 * Provides signal class templates that can pass arguments to signal handlers
 * connected via the slot interface (see scim_slot.h).
 *
 * The typed signal is the variadic template SignalN<R(Args...), Marshal>. The
 * historical fixed-arity names Signal0 .. Signal6 are kept as aliases for
 * source compatibility, where 0 to 6 specifies the number of arguments.
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
 * $Id: scim_signals.h,v 1.12 2005/01/30 13:24:13 suzhe Exp $
 */

#pragma once

#include <type_traits>

namespace scim {

/**
 * @addtogroup SignalSlot
 * @{
 */

class Signal;

//! @class SlotNode
//! @brief A node class for managing slots connected to scim::Signal's.

class SlotNode : public Node
{
    friend class Signal;

    SlotNode(Slot *slot);
    ~SlotNode();

    bool is_blocked;

public:
    bool blocked() const { return is_blocked; }
    //!< Returns true if the slot is blocked.

    virtual void block();
    //!< Block signal emission to the slot until unblock is called.

    virtual void unblock();
    //!< Unblock the slot so signal emmissions can be received.

    virtual void disconnect();
    //!< Disconnect the slot. The slot will no longer receive signal emissions.
};

// DefaultMarshal class (from marshal.h, libsigc++)

template <typename R>
class DefaultMarshal
{
public:
    typedef R OutType;
    typedef R InType;

private:
    OutType value_;

public:
    DefaultMarshal() :value_() {}

    OutType& value() { return value_; }

    // Return true to stop emission.
    bool marshal(const InType & newval)
    {
        value_ = newval;
        return false;
    }
};

// Marshal specialization
template <>
class DefaultMarshal <bool>
{
public:
    typedef bool OutType;
    typedef bool InType;

private:
    OutType value_;

public:
    DefaultMarshal() :value_(false) {}

    OutType& value() { return value_; }

    // Return true to stop emission.
    bool marshal(const InType & newval)
    {
        value_ = newval;
        return false;
    }
};

//! @class Signal
//! @brief Base class for the C++ signal interface.

class Signal
{
    Signal(const Signal&);
    Signal& operator=(const Signal&);

protected:
    typedef std::vector< Pointer<SlotNode> > ConnectionList;
    //!< ConnectionList type.

    ConnectionList connection_list;
    //!< A list of all the slots connected to the signal.

public:
    Signal();
    //!< Constructor.

    virtual ~Signal();
    //!< Destructor.

    SlotNode* connect(Slot *slot);
    //!< Creates a new SlotNode for slot and adds it to the <EM>connection_list</EM>.
};

//! Trait that recovers the default marshal type from a signal signature. Used
//! only to supply the SignalN default argument; the void case names
//! DefaultMarshal<void> but never instantiates it (void signals do not
//! marshal).

template <typename Sig>
struct signal_marshal_of;

template <typename R, typename... Args>
struct signal_marshal_of<R(Args...)>
{
    typedef DefaultMarshal<R> type;
};

//! @class SignalN
//! @brief A template for a signal passing Args... and returning R.
//!
//! The fixed-arity aliases Signal0 .. Signal6 below name specializations of
//! this template. The signature is given in function-type form, e.g.
//! SignalN<void(int, int)>, so that the Marshal policy can follow the argument
//! list.

template <typename Sig, typename Marshal = typename signal_marshal_of<Sig>::type>
class SignalN;

template <typename R, typename... Args, typename Marshal>
class SignalN<R(Args...), Marshal> : public Signal
{
    typedef SignalN<R(Args...), Marshal> Self;

    static R callback(void *data, Args... args)
    {
        return static_cast<Self*>(data)->emit(args...);
    }

public:
    typedef SlotN<R, Args...> SlotType;
    //!< Slot type for handlers connecting to the signal.

    Connection connect(SlotType *slot)
    {
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.

    SlotType* slot()
    {
        return new SignalSlotN<Self, R, Args...>(this, &callback);
    }
    //!< Returns a slot for this signal, so it can be connected to another one.

    R emit(Args... args)
    {
        if constexpr (std::is_void_v<R>) {
            ConnectionList::iterator i = connection_list.begin();
            while (i != connection_list.end()) {
                if (!(*i)->blocked()) {
                    SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                    if (slot) slot->call(args...);
                }
                ++i;
            }
        } else {
            Marshal m;
            ConnectionList::iterator i = connection_list.begin();
            while (i != connection_list.end()) {
                if (!(*i)->blocked()) {
                    SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                    if (slot && m.marshal(slot->call(args...)))
                        break;
                }
                ++i;
            }
            return m.value();
        }
    }
    //!< Emit the signal, calling every connected (unblocked) slot in order.

    R operator()(Args... args)
    {
        return emit(args...);
    }
    //!< Function operator; calls emit().
};

//
// Fixed-arity aliases kept for source compatibility. New code can use
// SignalN<R(Args...), Marshal> directly.
//

template <typename R, typename Marshal = DefaultMarshal<R> >
using Signal0 = SignalN<R(), Marshal>;

template <typename R, typename P1, typename Marshal = DefaultMarshal<R> >
using Signal1 = SignalN<R(P1), Marshal>;

template <typename R, typename P1, typename P2, typename Marshal = DefaultMarshal<R> >
using Signal2 = SignalN<R(P1, P2), Marshal>;

template <typename R, typename P1, typename P2, typename P3, typename Marshal = DefaultMarshal<R> >
using Signal3 = SignalN<R(P1, P2, P3), Marshal>;

template <typename R, typename P1, typename P2, typename P3, typename P4, typename Marshal = DefaultMarshal<R> >
using Signal4 = SignalN<R(P1, P2, P3, P4), Marshal>;

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename Marshal = DefaultMarshal<R> >
using Signal5 = SignalN<R(P1, P2, P3, P4, P5), Marshal>;

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename Marshal = DefaultMarshal<R> >
using Signal6 = SignalN<R(P1, P2, P3, P4, P5, P6), Marshal>;

/** @} */

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
