/*
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
 * $Id: scim_signals.cpp,v 1.7 2005/01/10 08:30:54 suzhe Exp $
 */

#define Uses_SCIM_OBJECT
#define Uses_SCIM_SIGNALS
#define Uses_SCIM_SLOT
#define Uses_SCIM_CONNECTION

#include "scim_private.h"
#include "scim.h"

namespace scim {

/*  SlotNode
 */
    
SlotNode::SlotNode(Slot *slot)
: Node(slot), is_blocked(false)
{
}

SlotNode::~SlotNode()
{
}

void 
SlotNode::block()
{
    is_blocked = true;
}
        
void 
SlotNode::unblock()
{ 
    is_blocked = false; 
}

void
SlotNode::disconnect()
{ 
    Node::disconnect ();
    is_blocked = true; 
}

/*  Signal;
 */

Signal::Signal()
{
}

Signal::~Signal()
{
}

SlotNode*
Signal::connect(Slot *slot)
{
    SlotNode *node = new SlotNode(slot);
    connection_list.push_back(node);
    return node;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
