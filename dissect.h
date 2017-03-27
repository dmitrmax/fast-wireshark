/*
 * This file is part of FAST Wireshark.
 *
 * FAST Wireshark is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FAST Wireshark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with FAST Wireshark.  If not, see
 * <http://www.gnu.org/licenses/lgpl.txt>.
 */

/*!
 * \file dissect.h
 * \brief  Handle payload dissection independent of Wireshark.
 *  Dissects the various data types supported by FAST protocol.
 *  Also has functions for dissecting various data types when operators
 *  (such as Delta, Copy, etc) are also invoked.
 *
 *  Functions...
 *    dissect_int_op
 *    dissect_ascii_delta
 *  ...are static, and located in dissect.c.
 */
#ifndef DISSECT_H_INCLUDED_
#define DISSECT_H_INCLUDED_

#include "basic-dissect.h"
#include "address-utils.h"

/*! \brief Dissect a FAST message by the bytes.
 * \param position  Current position in bytes.
 * \param parent  Return value. The message data is built under it.
 * \return  The template that was used to dissect.
 */
GNode* dissect_fast_bytes (wmem_map_t* templates, DissectPosition* position, GNode* parent, address* src, address* dest);

/*! \brief  Construct
 a message data tree (of FieldData).
 * \param tnode  Template node, contains type definition.
 * \param position  Current position in message.
 * \param parent  Parent node in data tree.
 * \param dnode  Previous node in data tree.
 */
void dissector_walk (const GNode* tnode,
                     DissectPosition* position,
                     GNode* parent, GNode* dnode, address* src, address* dest);

/*! \brief  Dissect a certain data type.
 * \param tnode  Template node, contains type definition.
 * \param position  Current position in message.
 * \param parent  Parent node in data tree.
 * \param dnode  Previous node in data tree.
 * \return  Node that was created.
 */
GNode* dissect_descend (const GNode* tnode,
                        DissectPosition* position,
                        GNode* parent, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect some value.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_value (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, check if an optional field is empty.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_optional (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief Given a byte stream with a copy operator, dissect it
 *            if it is used
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \return true if the copy operator is used
 */
gboolean dissect_copy (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief Given a byte stream with a default operator, dissect it
 *            if it is used
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \return true if the default operator is used
 */
gboolean dissect_default (const GNode* tnode,
                          DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect an unsigned 32bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_uint32 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect an unsigned 64bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_uint64 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect a signed 32bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_int32 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect a signed 64bit integer.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_int64 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Givdissect_ascii_deltaen a byte stream, dissect a decimal number.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_decimal (const GNode* tnode,
                      DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect an ASCII string.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_ascii_string (const GNode* tnode,
                           DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect a unicode string.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 * \sa dissect_byte_vector
 */
void dissect_unicode_string (const GNode* tnode,
                             DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect a byte vector.
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_byte_vector (const GNode* tnode,
                          DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect a group.
 *
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_group (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);

/*! \brief  Given a byte stream, dissect a sequence.
 *
 * \param tnode  Template tree node.
 * \param position  Position in the packet.
 * \param dnode  Dissect tree node.
 */
void dissect_sequence (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest);

#endif
