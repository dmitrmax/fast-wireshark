
#ifndef DISSECT_H_INCLUDED_
#define DISSECT_H_INCLUDED_

#include "basic-dissect.h"

/*! \brief  Hold a size prefixed data chunk. */
struct sized_data_struct
{
  guint nbytes;
  guint8* bytes;
};
typedef struct sized_data_struct SizedData;


const GNode* dissect_fast_bytes (guint nbytes, const guint8* bytes,
                                 GNode* parent);
void dissector_walk (const GNode* tnode,
                     DissectPosition* position,
                     GNode* parent, GNode* dnode);

GNode* dissect_type (const GNode* tnode,
                     DissectPosition* position,
                     GNode* parent, GNode* dnode);

void dissect_uint32 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode);
void dissect_uint64 (const GNode* tnode,
                     DissectPosition* position, GNode* dnode);
void dissect_int32 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode);
void dissect_int64 (const GNode* tnode,
                    DissectPosition* position, GNode* dnode);
void dissect_decimal (const GNode* tnode,
                      DissectPosition* position, GNode* dnode);
void dissect_ascii_string (const GNode* tnode,
                           DissectPosition* position, GNode* dnode);
void dissect_byte_vector (const GNode* tnode,
                          DissectPosition* position, GNode* dnode);
void dissect_group (const GNode* tnode,
                    DissectPosition* position, GNode* dnode);
void dissect_sequence (const GNode* tnode,
                       DissectPosition* position, GNode* dnode);

#endif

