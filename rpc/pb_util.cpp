#include <map>

#include "log.h"
#include "pb_util.h"
#include "wcf.pb.h"

#define BUF_SIZE (1024 * 1024)
static char buf[BUF_SIZE] = { 0 };

typedef std::map<int, std::string> MsgTypes_t;

void log_buffer(uint8_t *buffer, size_t len)
{
    size_t j = sprintf_s(buf, BUF_SIZE, "Encoded message[%ld]: ", len);
    for (size_t i = 0; i < len; i++) {
        j += sprintf_s(buf + j, BUF_SIZE, "%02X ", buffer[i]);
        if (j > BUF_SIZE - 3) {
            break;
        }
    }
    LOG_INFO(buf);
}

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    const char *str = (const char *)*arg;

    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }

    return pb_encode_string(stream, (uint8_t *)str, strlen(str));
}

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    std::string *str = static_cast<std::string *>(*arg);
    size_t len       = stream->bytes_left;
    str->resize(len);
    if (!pb_read(stream, (uint8_t *)str->data(), len)) {
        return false;
    }
    return true;
}

bool encode_types(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    MsgTypes_t *m               = (MsgTypes_t *)*arg;
    MsgTypes_TypesEntry message = MsgTypes_TypesEntry_init_default;

    for (auto it = m->begin(); it != m->end(); it++) {
        message.key                = it->first;
        message.value.funcs.encode = &encode_string;
        message.value.arg          = (void *)it->second.c_str();

        if (!pb_encode_tag_for_field(stream, field)) {
            return false;
        }

        if (!pb_encode_submessage(stream, MsgTypes_TypesEntry_fields, &message)) {
            return false;
        }
    }

    return true;
}
