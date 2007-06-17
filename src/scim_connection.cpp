/*
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
 * $Id: scim_connection.cpp,v 1.7 2005/01/10 08:30:53 suzhe Exp $
 */

#define Uses_SCIM_OBJECT
#define Uses_SCIM_CONNECTION
#define Uses_SCIM_SLOT
#include "scim_private.h"
#include "scim.h"

namespace scim {

/*  Node
 */

Node::Node(Slot *slot)
    : slot_(slot)
{
}
 
Node::~Node()
{
}

void
Node::disconnect()
{
    if (!slot_.null ()) slot_.reset ();
}

/*  Connection
 */

Connection::Connection()
    : node_(0)
{
}

Connection::Connection(Node *node)
    : node_(node) 
{
}

Connection::Connection(const Connection& src)
    : node_(src.node_)
{
}

Connection::~Connection()
{
}

Connection&
Connection::operator=(const Connection& src)
{
    if (src.node_ == node_)
        return *this;

    node_ = src.node_;
    src.node_.reset ();
    return *this;
}

void 
Connection::block()
{
    if (!node_.null ())
        node_->block();
}

void 
Connection::unblock()
{
    if (!node_.null ())
        node_->unblock();
}

void 
Connection::disconnect()
{
    if (!node_.null ()) {
        node_->disconnect();
        node_.reset ();
    }
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
