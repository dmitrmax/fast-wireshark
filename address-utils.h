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

#ifndef ADDRESS_H_INCLUDED_
#define ADDRESS_H_INCLUDED_

/* Due to wireshark crappyness the order of includes does matter */
#include <glib.h>
#include <epan/address.h>


guint addressHash( gconstpointer p);

gboolean addressEqual(gconstpointer a, gconstpointer b);

void addressDelete(gpointer data);

address* copyAddress(address* addr);

#endif