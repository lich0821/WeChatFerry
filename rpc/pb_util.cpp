#include "pb_util.h"
#include "log.h"
#include "pb_types.h"
#include "wcf.pb.h"

#define BUF_SIZE (1024 * 1024)
static char buf[BUF_SIZE] = { 0 };

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

bool encode_contacts(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    vector<RpcContact_t> *v = (vector<RpcContact_t> *)*arg;
    RpcContact message      = RpcContact_init_default;

    LOG_INFO("encode_contacts[{}]", v->size());
    for (auto it = v->begin(); it != v->end(); it++) {
        message.wxid.funcs.encode = &encode_string;
        message.wxid.arg          = (void *)(*it).wxid.c_str();

        message.code.funcs.encode = &encode_string;
        message.code.arg          = (void *)(*it).code.c_str();

        message.name.funcs.encode = &encode_string;
        message.name.arg          = (void *)(*it).name.c_str();

        message.country.funcs.encode = &encode_string;
        message.country.arg          = (void *)(*it).country.c_str();

        message.province.funcs.encode = &encode_string;
        message.province.arg          = (void *)(*it).province.c_str();

        message.city.funcs.encode = &encode_string;
        message.city.arg          = (void *)(*it).city.c_str();

        message.gender = (*it).gender;

        if (!pb_encode_tag_for_field(stream, field)) {
            return false;
        }

        if (!pb_encode_submessage(stream, RpcContact_fields, &message)) {
            return false;
        }
    }

    return true;
}
