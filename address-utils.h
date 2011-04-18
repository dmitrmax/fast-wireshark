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
* \file address-utils.h
* \brief  Converts wireshark macros into functions
*  Contains functions that are wrappers for macros in wireshark
*  so that they can be passed in as function pointers
*
*  Functions:
*     addressHash
*     addressEqual
*     addressDelete
*     copyAddress
*/

#ifndef ADDRESS_H_INCLUDED_
#define ADDRESS_H_INCLUDED_

/* Due to wireshark crappyness the order of includes does matter */
#include <glib.h>
#include <epan/address.h>

/*! \brief function wrapper for wireshark macro ADD_ADDRESS_TO_HASH
 *  \param p address pointer to be added
 *  \return guint return value from ADD_ADDRESS_TO_HASH
 */
guint addressHash( gconstpointer p);

/*! \brief function wrapper for wireshark macro ADDRESSES_EQUAL
*  \param a first address to be compared
*  \param b second address to be compared
*  \return true if the addresses are equal
*/
gboolean addressEqual(gconstpointer a, gconstpointer b);

/*! \brief frees memory located at given address
*  \param data gpointer pointing to data to be deleted
*/
void addressDelete(gpointer data);

/*! \brief function wrapper for wireshark macro COPY_ADDRESS
*  \param addr address pointer of data to be copied
*  \return address pointer to new copy
*/
address* copyAddress(address* addr);

#endif