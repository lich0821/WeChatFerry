#include "pb_util.h"
#include "log.h"
#include "pb_types.h"
#include "wcf.pb.h"

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    const char *str = (const char *)*arg;

    if (!pb_encode_tag_for_field(stream, field)) {
        LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
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
        LOG_ERROR("Decoding failed: {}", PB_GET_ERROR(stream));
        return false;
    }
    return true;
}

bool encode_bytes(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    vector<uint8_t> *v = (vector<uint8_t> *)*arg;

    if (!pb_encode_tag_for_field(stream, field)) {
        LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
        return false;
    }

    return pb_encode_string(stream, (uint8_t *)v->data(), v->size());
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
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }

        if (!pb_encode_submessage(stream, MsgTypes_TypesEntry_fields, &message)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }
    }

    return true;
}

bool encode_contacts(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    vector<RpcContact_t> *v = (vector<RpcContact_t> *)*arg;
    RpcContact message      = RpcContact_init_default;

    LOG_DEBUG("encode_contacts[{}]", v->size());
    for (auto it = v->begin(); it != v->end(); it++) {
        message.wxid.funcs.encode = &encode_string;
        message.wxid.arg          = (void *)(*it).wxid.c_str();

        message.code.funcs.encode = &encode_string;
        message.code.arg          = (void *)(*it).code.c_str();

        message.remark.funcs.encode = &encode_string;
        message.remark.arg          = (void *)(*it).remark.c_str();

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
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }

        if (!pb_encode_submessage(stream, RpcContact_fields, &message)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }
    }

    return true;
}

bool encode_dbnames(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    DbNames_t *v = (DbNames_t *)*arg;
    for (auto it = v->begin(); it != v->end(); it++) {
        if (!pb_encode_tag_for_field(stream, field)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }

        if (!pb_encode_string(stream, (uint8_t *)(*it).c_str(), (*it).size())) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }
    }

    return true;
}

bool encode_tables(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    DbTables_t *v   = (DbTables_t *)*arg;
    DbTable message = DbTable_init_default;

    for (auto it = v->begin(); it != v->end(); it++) {
        message.name.funcs.encode = &encode_string;
        message.name.arg          = (void *)(*it).name.c_str();

        message.sql.funcs.encode = &encode_string;
        message.sql.arg          = (void *)(*it).sql.c_str();

        if (!pb_encode_tag_for_field(stream, field)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }

        if (!pb_encode_submessage(stream, DbTable_fields, &message)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }
    }

    return true;
}

static bool encode_fields(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    DbRow_t *v      = (DbRow_t *)*arg;
    DbField message = DbField_init_default;

    for (auto it = v->begin(); it != v->end(); it++) {
        message.type = (*it).type;

        message.column.arg          = (void *)(*it).column.c_str();
        message.column.funcs.encode = &encode_string;

        message.content.arg          = (void *)&(*it).content;
        message.content.funcs.encode = &encode_bytes;

        if (!pb_encode_tag_for_field(stream, field)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }

        if (!pb_encode_submessage(stream, DbField_fields, &message)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }
    }

    return true;
}

bool encode_rows(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    DbRows_t *v   = (DbRows_t *)*arg;
    DbRow message = DbRow_init_default;

    for (auto it = v->begin(); it != v->end(); it++) {
        message.fields.arg          = (void *)&(*it);
        message.fields.funcs.encode = &encode_fields;

        if (!pb_encode_tag_for_field(stream, field)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }

        if (!pb_encode_submessage(stream, DbRow_fields, &message)) {
            LOG_ERROR("Encoding failed: {}", PB_GET_ERROR(stream));
            return false;
        }
    }

    return true;
}
