/** @file scim_connection.h
 * @brief C++ signal-slot connection interface.
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
 * $Id: scim_connection.h,v 1.10 2005/01/10 08:30:53 suzhe Exp $
 */

#ifndef __SCIM_CONNECTION_H
#define __SCIM_CONNECTION_H

namespace scim {

/**
 * @addtogroup SignalSlot
 * @{
 */

class Slot;

/**
 * @class Node 
 * @brief Base class for classes managing slots.
 * 
 * A node connects a slot to its Connection class, the class returned
 * from a signal's connect() method.
 */

class Node : public ReferencedObject
{
    Pointer <Slot> slot_;

protected:
    Node(Slot *slot);
    //!< Constructor.

    virtual ~Node();
    //!< Destructor.

public:
    Slot* slot() { return slot_.get (); }
    //!< Returns a pointer to the slot held by this node.

    virtual void block() = 0;
    //!< Block signal emission to the slot until unblock is called.

    virtual void unblock() = 0;
    //!< Unblock the slot so signal emmissions can be received.

    virtual void disconnect() = 0;
    //!< Disconnect the slot. The slot will no longer receive signal emissions.
};

/**
 * @class Connection 
 * @brief A signal connection class.
 * 
 * A Connection class is returned by value from a signal's connect()
 * method. Using this class you can block, unblock and disconnect
 * a signal connection.
 */

class Connection
{
    mutable Pointer<Node> node_;

public:
//! @name Constructors
//! @{

    Connection();
    //!< Default constructor.

    Connection(Node *node);
    //!< Construct a connection object for node.
    //!< @param node - a pointer to the Node class for this connection.

    Connection(const Connection& src);
    //!< Copy constructor.

    ~Connection();
    //!< Destructor.

    Connection& operator=(const Connection& src);
    //!< Assignment operator.
    
//! @}
//! @name Methods
//! @{

    void block();
    //!< Block signal transmission to a slot.
    //!< The slot will not be called during any signal emissions unless it is unblocked again.

    void unblock();
    //!< Unblock a previously blocked slot.
    //!< A blocked slot is skipped during signal emissions and will not be invoked, unblocking
    //!< it (for exactly the number of times it has been blocked before) reverts its "blocked"
    //!< state, so the slot will be recognized by the signal system and is called upon future
    //!< or currently ongoing signal emissions.

    void disconnect();
    //!< Disconnect a slot.
    //!< The slot will not be called during any future or currently ongoing
    //!< emissions of the signal it has been connected to.
    
//! @}
};

/** @} */

} // namespace scim

#endif //__SCIM_CONNECTION_H

/*
vi:ts=4:nowrap:ai:expandtab
*/

