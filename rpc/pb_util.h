#pragma once

#include <pb_decode.h>
#include <pb_encode.h>

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);
bool encode_types(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool encode_contacts(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool encode_dbnames(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool encode_tables(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool encode_rows(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
