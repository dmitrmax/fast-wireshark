
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