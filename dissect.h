
#ifndef DISSECT_H_INCLUDED_
#define DISSECT_H_INCLUDED_

#include "basic-dissect.h"
#include "address-utils.h"

GNode* dissect_fast_bytes (DissectPosition* position, GNode* parent, address* src, address* dest);
void dissector_walk (const GNode* tnode,
                     DissectPosition* position,
                     GNode* parent, GNode* dnode, address* src, address* dest);

GNode* dissect_descend (const GNode* tnode,
                        DissectPosition* position,
                        GNode* parent, GNode* dnode, address* src, address* dest);
void dissect_value (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_optional (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest);
gboolean dissect_copy (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest);
gboolean dissect_default (const GNode* tnode,
                          DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_uint32 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_uint64 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_int32 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_int64 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_decimal (const GNode* tnode,
                      DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_ascii_string (const GNode* tnode,
                           DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_unicode_string (const GNode* tnode,
                             DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_byte_vector (const GNode* tnode,
                          DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_group (const GNode* tnode,
                    DissectPosition* position, GNode* dnode, address* src, address* dest);
void dissect_sequence (const GNode* tnode,
                       DissectPosition* position, GNode* dnode, address* src, address* dest);

#endif

