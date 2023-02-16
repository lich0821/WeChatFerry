#pragma once

#include <pb_decode.h>
#include <pb_encode.h>

void log_buffer(uint8_t *buffer, size_t len);
bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);
bool encode_types(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
bool encode_contacts(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
