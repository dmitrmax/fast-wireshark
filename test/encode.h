
#ifndef ENCODE_H_
#define ENCODE_H_

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

void encode_pmap (const GByteArray* pmap, GByteArray** pto_a);
void encode_uint32 (guint32 x, GByteArray** arr);
void encode_int32 (gint32 x, GByteArray** arr);

#endif

