#pragma once

#include <cstddef>
#include <cstdint>

#include "pb_types.h"

namespace message
{

MsgTypes_t get_msg_types();
bool rpc_get_msg_types(uint8_t *out, size_t *len);
bool rpc_enable_recv_txt(bool pyq, uint8_t *out, size_t *len);
bool rpc_disable_recv_txt(uint8_t *out, size_t *len);
void stop_receiving();

} // namespace message
