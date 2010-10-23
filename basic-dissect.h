
#ifndef BASIC_DISSECT_H_INCLUDED_
#define BASIC_DISSECT_H_INCLUDED_

#include "basic-field.h"

/*! \brief  Identify the position of this field in the stream.
 */
struct field_data_struct
{
  guint start;
  guint nbytes;
  gboolean empty;
  FieldValue value;
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


void ShiftBytes(DissectPosition* position);
gboolean dissect_shift_pmap (DissectPosition* position);
gboolean dissect_shift_null (DissectPosition* position);

void basic_dissect_uint32 (DissectPosition* position, FieldData* fdata);
void basic_dissect_uint64 (DissectPosition* position, FieldData* fdata);
void basic_dissect_int32 (DissectPosition* position, FieldData* fdata);
void basic_dissect_int64 (DissectPosition* position, FieldData* fdata);
void basic_dissect_ascii_string (DissectPosition* position, FieldData* fdata);

#endif

