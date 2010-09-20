
#ifndef DISSECT_H_INCLUDED_
#define DISSECT_H_INCLUDED_

#include <gmodule.h>

/*! \brief  Identify the position of this field in the stream.
 */
struct field_data_struct
{
  guint start;
  guint nbytes;
  void* value;
};
typedef struct field_data_struct FieldData;


/*! \brief  Hold current dissection state/position. */
struct dissect_position_struct
{
  guint offjmp; /* Number of bytes to the next offset. */
  guint offset; /* Current offset in the byte array. */
  guint nbytes;
  const guint8* bytes;

  guint pmap_len;
  guint pmap_idx;
  gboolean* pmap;
};
typedef struct dissect_position_struct DissectPosition;


gboolean dissect_shift_pmap (DissectPosition* position);
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

#endif

