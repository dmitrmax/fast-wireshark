#include <string.h>
#include "address-utils.h"
#include "debug.h"


guint addressHash( gconstpointer p){
  guint ret = 0;
  address* addr = (address*)p;
  if(!addr){BAILOUT(0,"NULL Pointer\n");}
  ADD_ADDRESS_TO_HASH(ret, addr);
  return ret;
}

gboolean addressEqual(gconstpointer a, gconstpointer b){
  address* addr1 = (address*)a;
  address* addr2 = (address*)b;
  if(!addr1){BAILOUT(0,"NULL Pointer\n");}
  if(!addr2){BAILOUT(0,"NULL Pointer\n");}
  return ADDRESSES_EQUAL(addr1, addr2);  
}

void addressDelete(gpointer p){
  address* addr = (address*)p;
  if(!addr){BAILOUT_VOID("NULL Pointer\n");}
  g_free((void*)addr->data);
  g_free(addr);
}

address* copyAddress(address* addr){
  address* addre;
  addre = (address*)g_malloc(sizeof(address));
  COPY_ADDRESS(addre, addr);
  return addre;
}
  
  