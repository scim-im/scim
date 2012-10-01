/**
 * @file scim_signals.h
 * @brief C++ signal interface.
 * 
 * Provides a set of signal class templates you can use to create signals
 * that can pass up to 6 arguments to signal handlers connected via the
 * slot interface (see scim_slot.h). The signal classes are named Signal0
 * to Signal6, where 0 to 6 specifies the number of arguments that can be
 * passed to a slot.
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
 * $Id: scim_signals.h,v 1.12 2005/01/30 13:24:13 suzhe Exp $
 */

#ifndef __SCIM_SIGNALS_H
#define __SCIM_SIGNALS_H

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

//! @class Signal0 
//! @brief A template for a signal passing no arguments and returning a value of type R.

template<typename R, typename Marshal = class DefaultMarshal<R> >
class Signal0 : public Signal
{
    typedef Signal0<R> Self;

    static R callback(void *data)
    {
        Self *s = static_cast<Self*>(data);
        return s->emit();
    }

public:
    typedef Slot0<R> SlotType;
    //!< Function signature for handlers connecting the signal.

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.
    //!< @param slot - a slot of type Slot0<R>.
    //!< @return a connection object.
    //!<
    //!< <BR>The returned connection object can be used alter or change the connection.

    SlotType* slot()
    {
        return new SignalSlot0<Self, R>(this, &callback);
    }
    //!< Returns a slot for this signal.
    //!< @return a new slot of type Slot0<R>.
    //!<
    //!< <BR>The returned slot can be passed to another signal allowing the
    //!< other signal to call this signal when it gets emitted.

    R emit()
    {
        Marshal m;
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot && m.marshal(slot->call()))
                    break;
            }
            ++i;
        }
        return m.value();    
    }
    //!< Emit the signal.
    //!< @return the value returned by the signal handler.
    //!<
    //!< <BR>Calls every slot connected to this signal, in order of connection.

    R operator()() 
    {
        return emit(); 
    }
    //!< Function operator; calls emit().
};

// Signal0 partially specialized for void return

template<typename IgnoreMarshal>
class Signal0<void, IgnoreMarshal> : public Signal
{
    typedef Signal0<void, IgnoreMarshal> Self;

    static void callback(void *data)
    {
        Self *s = static_cast<Self*>(data);
        s->emit();
    }

public:
    typedef Slot0<void> SlotType;

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }

    SlotType* slot()
    {
        return new SignalSlot0<Self, void>(this, &callback);
    }

    void emit()
    {
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot) slot->call();
            }
            ++i;
        }
    }
        
    void operator()()
    {
        emit(); 
    }
};

//! @class Signal1 
//! @brief A template for a signal passing one argument of type P1 and returning a value of type R.

template<typename R, typename P1, typename Marshal = class DefaultMarshal<R> >
class Signal1 : public Signal
{
    typedef Signal1<R, P1> Self;

    static R callback(void *data, P1 p1)
    {
        Self *s = static_cast<Self*>(data);
        return s->emit(p1);
    }

public:
    typedef Slot1<R, P1> SlotType;
    //!< Function signature for handlers connecting to the signal.

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.
    //!< @param slot - a slot of type Slot1<R, P1>.
    //!< @return a connection object.
    //!<
    //!< <BR>The returned connection object can be used alter or change the connection.

    SlotType* slot()
    {
        return new SignalSlot1<Self, R, P1>(this, &callback);
    }
    //!< Returns a slot for this signal.
    //!< @return a new slot of type Slot1<R, P1>.
    //!<
    //!< <BR>The returned slot can be passed to another signal allowing the
    //!< other signal to call this signal when it gets emitted.

    R emit(P1 p1)
    {
        Marshal m;
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot && m.marshal(slot->call(p1)))
                    break;
            }
            ++i;        
        }
        return m.value();    
    }
    //!< Emit the signal.
    //!< @param p1 - passes p1 to the signal handler.
    //!< @return the value returned by the signal handler.
    //!<
    //!< <BR>Calls every slot connected to this signal, in order of connection.

    R operator()(P1 p1) 
    {
        return emit(p1); 
    }
    //!< Function operator; calls emit().
};

// Signal1 partially specialized for void return

template<typename P1, typename IgnoreMarshal>
class Signal1<void, P1, IgnoreMarshal> : public Signal
{
    typedef Signal1<void, P1, IgnoreMarshal> Self;

    static void callback(void *data, P1 p1)
    {
        Self *s = static_cast<Self*>(data);
        s->emit(p1);
    }

public:
    typedef Slot1<void, P1> SlotType;

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }

    SlotType* slot()
    {
        return new SignalSlot1<Self, void, P1>(this, &callback);
    }

    void emit(P1 p1)
    {
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot) slot->call(p1);
            }
            ++i;
        }
    }
        
    void operator()(P1 p1)
    {
        emit(p1); 
    }
};

//! @class Signal2 
//! @brief A template for a signal passing two arguments of type P1 and P2,
//! and returning a value of type R.

template<typename R, typename P1, typename P2, typename Marshal = class DefaultMarshal<R> >
class Signal2 : public Signal
{
    typedef Signal2<R, P1, P2> Self;

    static R callback(void *data, P1 p1, P2 p2)
    {
        Self *s = static_cast<Self*>(data);
        return s->emit(p1, p2);
    }

public:
    typedef Slot2<R, P1, P2> SlotType;
    //!< Function signature for handlers connecting to the signal.

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.
    //!< @param slot - a slot of type Slot2<R, P1, P2>.
    //!< @return a connection object.
    //!<
    //!< <BR>The returned connection object can be used alter or change the connection.

    SlotType* slot()
    {
        return new SignalSlot2<Self, R, P1, P2>(this, &callback);
    }
    //!< Returns a slot for this signal.
    //!< @return a new slot of type Slot2<R, P1, P2>.
    //!<
    //!< <BR>The returned slot can be passed to another signal allowing the
    //!< other signal to call this signal when it gets emitted.

    R emit(P1 p1, P2 p2)
    {
        Marshal m;
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot && m.marshal(slot->call(p1, p2)))
                    break;
            }
            ++i;        
        }
        return m.value();    
    }
    //!< Emit the signal.
    //!< @param p1 - passes p1 to the signal handler.
    //!< @param p2 - passes p2 to the signal handler.
    //!< @return the value returned by the signal handler.
    //!<
    //!< <BR>Calls every slot connected to this signal, in order of connection.

    R operator()(P1 p1, P2 p2) 
    {
        return emit(p1, p2); 
    }
    //!< Function operator; calls emit().
};

// Signal2 partially specialized for void return

template<typename P1, typename P2, typename IgnoreMarshal>
class Signal2<void, P1, P2, IgnoreMarshal> : public Signal
{
    typedef Signal2<void, P1, P2, IgnoreMarshal> Self;

    static void callback(void *data, P1 p1, P2 p2)
    {
        Self *s = static_cast<Self*>(data);
        s->emit(p1, p2);
    }

public:
    typedef Slot2<void, P1, P2> SlotType;

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }

    SlotType* slot()
    {
        return new SignalSlot2<Self, void, P1, P2>(this, &callback);
    }

    void emit(P1 p1, P2 p2)
    {
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot) slot->call(p1, p2);
            }
            ++i;
        }
    }
        
    void operator()(P1 p1, P2 p2)
    {
        emit(p1, p2); 
    }
};

//! @class Signal3 
//! @brief A template for a signal passing three arguments of type P1, P2 and P3,
//! and returning a value of type R.

template<typename R, typename P1, typename P2, typename P3, typename Marshal = class DefaultMarshal<R> >
class Signal3 : public Signal
{
    typedef Signal3<R, P1, P2, P3> Self;

    static R callback(void *data, P1 p1, P2 p2, P3 p3)
    {
        Self *s = static_cast<Self*>(data);
        return s->emit(p1, p2, p3);
    }

public:
    typedef Slot3<R, P1, P2, P3> SlotType;
    //!< Function signature for handlers connecting to the signal.

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.
    //!< @param slot - a slot of type Slot3<R, P1, P2, P3>.
    //!< @return a connection object.
    //!<
    //!< <BR>The returned connection object can be used alter or change the connection.

    SlotType* slot()
    {
        return new SignalSlot3<Self, R, P1, P2, P3>(this, &callback);
    }
    //!< Returns a slot for this signal.
    //!< @return a new slot of type Slot3<R, P1, P2, P3>.
    //!<
    //!< <BR>The returned slot can be passed to another signal allowing the
    //!< other signal to call this signal when it gets emitted.

    R emit(P1 p1, P2 p2, P3 p3)
    {
        Marshal m;
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot && m.marshal(slot->call(p1, p2, p3)))
                    break;
            }
            ++i;        
        }
        return m.value();    
    }
    //!< Emit the signal.
    //!< @param p1 - passes p1 to the signal handler.
    //!< @param p2 - passes p2 to the signal handler.
    //!< @param p3 - passes p3 to the signal handler.
    //!< @return the value returned by the signal handler.
    //!<
    //!< <BR>Calls every slot connected to this signal, in order of connection.

    R operator()(P1 p1, P2 p2, P3 p3) 
    {
        return emit(p1, p2, p3); 
    }
    //!< Function operator; calls emit().
};

// Signal3 partially specialized for void return

template<typename P1, typename P2, typename P3, typename IgnoreMarshal>
class Signal3<void, P1, P2, P3, IgnoreMarshal> : public Signal
{
    typedef Signal3<void, P1, P2, P3, IgnoreMarshal> Self;

    static void callback(void *data, P1 p1, P2 p2, P3 p3)
    {
        Self *s = static_cast<Self*>(data);
        s->emit(p1, p2, p3);
    }

public:
    typedef Slot3<void, P1, P2, P3> SlotType;

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }

    SlotType* slot()
    {
        return new SignalSlot3<Self, void, P1, P2, P3>(this, &callback);
    }

    void emit(P1 p1, P2 p2, P3 p3)
    {
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot) slot->call(p1, p2, p3);
            }
            ++i;
        }
    }
        
    void operator()(P1 p1, P2 p2, P3 p3)
    {
        emit(p1, p2, p3); 
    }
};

//! @class Signal4 
//! @brief A template for a signal passing four arguments of type P1, P2, P3 and P4,
//! and returning a value of type R.

template<typename R, typename P1, typename P2, typename P3, typename P4, typename Marshal = class DefaultMarshal<R> >
class Signal4 : public Signal
{
    typedef Signal4<R, P1, P2, P3, P4> Self;

    static R callback(void *data, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        Self *s = static_cast<Self*>(data);
        return s->emit(p1, p2, p3, p4);
    }

public:
    typedef Slot4<R, P1, P2, P3, P4> SlotType;
    //!< Function signature for handlers connecting to the signal.

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.
    //!< @param slot - a slot of type Slot4<R, P1, P2, P3, P4>.
    //!< @return a connection object.
    //!<
    //!< <BR>The returned connection object can be used alter or change the connection.

    SlotType* slot()
    {
        return new SignalSlot4<Self, R, P1, P2, P3, P4>(this, &callback);
    }
    //!< Returns a slot for this signal.
    //!< @return a new slot of type Slot4<R, P1, P2, P3, P4>.
    //!<
    //!< <BR>The returned slot can be passed to another signal allowing the
    //!< other signal to call this signal when it gets emitted.

    R emit(P1 p1, P2 p2, P3 p3, P4 p4)
    {
        Marshal m;
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot && m.marshal(slot->call(p1, p2, p3, p4)))
                    break;
            }
            ++i;        
        }
        return m.value();    
    }
    //!< Emit the signal.
    //!< @param p1 - passes p1 to the signal handler.
    //!< @param p2 - passes p2 to the signal handler.
    //!< @param p3 - passes p3 to the signal handler.
    //!< @param p4 - passes p4 to the signal handler.
    //!< @return the value returned by the signal handler.
    //!<
    //!< <BR>Calls every slot connected to this signal, in order of connection.

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4) 
    {
        return emit(p1, p2, p3, p4); 
    }
    //!< Function operator; calls emit().
};

// Signal4 partially specialized for void return

template<typename P1, typename P2, typename P3, typename P4, typename IgnoreMarshal>
class Signal4<void, P1, P2, P3, P4, IgnoreMarshal> : public Signal
{
    typedef Signal4<void, P1, P2, P3, P4, IgnoreMarshal> Self;

    static void callback(void *data, P1 p1, P2 p2, P3 p3, P4 p4)
    {
        Self *s = static_cast<Self*>(data);
        s->emit(p1, p2, p3, p4);
    }

public:
    typedef Slot4<void, P1, P2, P3, P4> SlotType;

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }

    SlotType* slot()
    {
        return new SignalSlot4<Self, void, P1, P2, P3, P4>(this, &callback);
    }

    void emit(P1 p1, P2 p2, P3 p3, P4 p4)
    {
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot) slot->call(p1, p2, p3, p4);
            }
            ++i;
        }
    }
        
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4)
    {
        emit(p1, p2, p3, p4); 
    }
};

//! @class Signal5 
//! @brief A template for a signal passing five arguments of type P1, P2, P3, P4 and P5,
//! and returning a value of type R.

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename Marshal = class DefaultMarshal<R> >
class Signal5 : public Signal
{
    typedef Signal5<R, P1, P2, P3, P4, P5> Self;

    static R callback(void *data, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        Self *s = static_cast<Self*>(data);
        return s->emit(p1, p2, p3, p4, p5);
    }

public:
    typedef Slot5<R, P1, P2, P3, P4, P5> SlotType;
    //!< Function signature for handlers connecting to the signal.

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.
    //!< @param slot - a slot of type Slot5<R, P1, P2, P3, P4, P5>.
    //!< @return a connection object.
    //!<
    //!< <BR>The returned connection object can be used alter or change the connection.

    SlotType* slot()
    {
        return new SignalSlot5<Self, R, P1, P2, P3, P4, P5>(this, &callback);
    }
    //!< Returns a slot for this signal.
    //!< @return a new slot of type Slot5<R, P1, P2, P3, P4, P5>.
    //!<
    //!< <BR>The returned slot can be passed to another signal allowing the
    //!< other signal to call this signal when it gets emitted.

    R emit(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        Marshal m;
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot && m.marshal(slot->call(p1, p2, p3, p4, p5)))
                    break;
            }
            ++i;        
        }
        return m.value();    
    }
    //!< Emit the signal.
    //!< @param p1 - passes p1 to the signal handler.
    //!< @param p2 - passes p2 to the signal handler.
    //!< @param p3 - passes p3 to the signal handler.
    //!< @param p4 - passes p4 to the signal handler.
    //!< @param p5 - passes p5 to the signal handler.
    //!< @return the value returned by the signal handler.
    //!<
    //!< <BR>Calls every slot connected to this signal, in order of connection.

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) 
    {
        return emit(p1, p2, p3, p4, p5); 
    }
    //!< Function operator; calls emit().
};

// Signal5 partially specialized for void return

template<typename P1, typename P2, typename P3, typename P4, typename P5, typename IgnoreMarshal>
class Signal5<void, P1, P2, P3, P4, P5, IgnoreMarshal> : public Signal
{
    typedef Signal5<void, P1, P2, P3, P4, P5, IgnoreMarshal> Self;

    static void callback(void *data, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        Self *s = static_cast<Self*>(data);
        s->emit(p1, p2, p3, p4, p5);
    }

public:
    typedef Slot5<void, P1, P2, P3, P4, P5> SlotType;

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }

    SlotType* slot()
    {
        return new SignalSlot5<Self, void, P1, P2, P3, P4, P5>(this, &callback);
    }

    void emit(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot) slot->call(p1, p2, p3, p4, p5);
            }
            ++i;
        }
    }
        
    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
        emit(p1, p2, p3, p4, p5); 
    }
};

//! @class Signal6 
//! @brief A template for a signal passing six arguments of type P1, P2, P3, P4, P5 and P6,
//! and returning a value of type R.

template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename Marshal = class DefaultMarshal<R> >
class Signal6 : public Signal
{
    typedef Signal6<R, P1, P2, P3, P4, P5, P6> Self;

    static R callback(void *data, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        Self *s = static_cast<Self*>(data);
        return s->emit(p1, p2, p3, p4, p5, p6);
    }

public:
    typedef Slot6<R, P1, P2, P3, P4, P5, P6> SlotType;
    //!< Function signature for handlers connecting to the signal.

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }
    //!< Connect a slot to the signal.
    //!< @param slot - a slot of type Slot6<R, P1, P2, P3, P4, P5, P6>.
    //!< @return a connection object.
    //!<
    //!< <BR>The returned connection object can be used alter or change the connection.

    SlotType* slot()
    {
        return new SignalSlot6<Self, R, P1, P2, P3, P4, P5, P6>(this, &callback);
    }
    //!< Returns a slot for this signal.
    //!< @return a new slot of type Slot6<R, P1, P2, P3, P4, P5, P6>.
    //!<
    //!< <BR>The returned slot can be passed to another signal allowing the
    //!< other signal to call this signal when it gets emitted.

    R emit(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        Marshal m;
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot && m.marshal(slot->call(p1, p2, p3, p4, p5, p6)))
                    break;
            }
            ++i;        
        }
        return m.value();    
    }
    //!< Emit the signal.
    //!< @param p1 - passes p1 to the signal handler.
    //!< @param p2 - passes p2 to the signal handler.
    //!< @param p3 - passes p3 to the signal handler.
    //!< @param p4 - passes p4 to the signal handler.
    //!< @param p5 - passes p5 to the signal handler.
    //!< @param p6 - passes p6 to the signal handler.
    //!< @return the value returned by the signal handler.
    //!<
    //!< <BR>Calls every slot connected to this signal, in order of connection.

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) 
    {
        return emit(p1, p2, p3, p4, p5, p6); 
    }
    //!< Function operator; calls emit().
};

/*  Signal6 partially specialized for void return
 */
 
template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename IgnoreMarshal>
class Signal6<void, P1, P2, P3, P4, P5, P6, IgnoreMarshal> : public Signal
{
    typedef Signal6<void, P1, P2, P3, P4, P5, P6, IgnoreMarshal> Self;

    static void callback(void *data, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        Self *s = static_cast<Self*>(data);
        s->emit(p1, p2, p3, p4, p5, p6);
    }

public:
    typedef Slot6<void, P1, P2, P3, P4, P5, P6> SlotType;

    Connection connect(SlotType *slot)
    { 
        return Signal::connect(slot);
    }

    SlotType* slot()
    {
        return new SignalSlot6<Self, void, P1, P2, P3, P4, P5, P6>(this, &callback);
    }

    void emit(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        ConnectionList::iterator i = connection_list.begin();
        while (i != connection_list.end())
        {
            if (!(*i)->blocked())
            {
                SlotType *slot = dynamic_cast<SlotType*>((*i)->slot());
                if (slot) slot->call(p1, p2, p3, p4, p5, p6);
            }
            ++i;
        }
    }

    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
        emit(p1, p2, p3, p4, p5, p6); 
    }
};

/** @} */

} // namespace scim

#endif //__SCIM_SIGNALS_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

