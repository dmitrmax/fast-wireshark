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

#include <string.h>
#include "address-utils.h"
#include "debug.h"

guint addressHash( gconstpointer p){
  guint ret = 0;
  address* addr = (address*)p;
  if(!addr){BAILOUT(0,"NULL Pointer\n");}
  add_address_to_hash(ret, addr);
  return ret;
}

gboolean addressEqual(gconstpointer a, gconstpointer b){
  address* addr1 = (address*)a;
  address* addr2 = (address*)b;
  if(!addr1){BAILOUT(0,"NULL Pointer\n");}
  if(!addr2){BAILOUT(0,"NULL Pointer\n");}
  return addresses_equal(addr1, addr2);
}

void addressDelete(gpointer p){
  address* addr = (address*)p;
  if(!addr){BAILOUT_VOID("NULL Pointer\n");}
  g_free((void*)addr->data);
  g_free(addr);
}

address* copyAddress(address* addr){
  address* addr_copy;
  addr_copy = (address*)g_malloc(sizeof(address));
  copy_address(addr_copy, addr);
  return addr_copy;
}
