#pragma once

#include <cstdint>
namespace xrtc {
const int kHeadSize = 36;
const uint32_t kHeadMagicNum = 0x11451419;

struct tcp_head_t {
    uint16_t id;
    uint16_t version;
    uint32_t log_id;
    char provider[16];
    uint32_t magic_num;
    uint32_t reserved;
    uint32_t body_len;
};

} // namespace xrtc