// 包含RTP包的头文件定义
#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet.h"

// 包含WebRTC日志功能的头文件
#include <rtc_base/logging.h>
// 包含字节操作工具的头文件，用于大小端转换
#include "modules/rtp_rtcp/source/byte_io.h"

namespace xrtc {

// 默认的RTP包容量，通常为以太网MTU大小
const size_t kDefaultCapacity = 1500;
// RTP固定头部大小为12字节
const size_t kFixedHeaderSize = 12;
// RTP协议版本号，当前为2
const uint8_t kRtpVersion = 2;

//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |V=2|P|X|  CC   |M|     PT      |       sequence number         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           timestamp                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           synchronization source (SSRC) identifier            |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |            Contributing source (CSRC) identifiers             |
// |                             ....                              |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |  header eXtension profile id  |       length in 32bits        |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          Extensions                           |
// |                             ....                              |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |                           Payload                             |
// |             ....              :  padding...                   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |               padding         | Padding size  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// 默认构造函数，调用带参数的构造函数并使用默认容量
RtpPacket::RtpPacket() : RtpPacket(kDefaultCapacity) {
}

// 带容量参数的构造函数，初始化缓冲区并调用Clear()初始化其他字段
RtpPacket::RtpPacket(size_t capacity) :
    buffer_(capacity)
{
    Clear();
}

// 清空RTP包，重置所有字段为初始值
void RtpPacket::Clear() {
    // 重置标记位为false
    marker_ = false;
    // 重置负载类型为0
    payload_type_ = 0;
    // 重置序列号为0
    sequence_number_ = 0;
    // 重置时间戳为0
    timestamp_ = 0;
    // 重置SSRC为0
    ssrc_ = 0;
    // 设置负载起始位置为固定头部大小
    payload_offset_ = kFixedHeaderSize;
    // 重置负载大小为0
    payload_size_ = 0;
    // 重置填充大小为0
    padding_size_ = 0;

    // 设置缓冲区大小为固定头部大小
    buffer_.SetSize(kFixedHeaderSize);
    // 写入RTP版本信息到第一个字节的高2位
    WriteAt(0, kRtpVersion << 6);
}

// 设置RTP包的标记位
void RtpPacket::SetMarker(bool marker_bit) {
    // 更新成员变量
    marker_ = marker_bit;
    if (marker_bit) {
        // 如果设置标记位，将第2个字节的最高位设为1
        WriteAt(1, data()[1] | 0x80);
    }
    else {
        // 如果清除标记位，将第2个字节的最高位设为0
        WriteAt(1, data()[1] & 0x7F);
    }
}

// 设置RTP包的负载类型
void RtpPacket::SetPayloadType(uint8_t payload_type) {
    // 更新成员变量
    payload_type_ = payload_type;
    // 保留第2个字节的最高位(M位)，更新低7位为负载类型
    WriteAt(1, (data()[1] & 0x80) | payload_type);
}

// 设置RTP包的序列号
void RtpPacket::SetSequenceNumber(uint16_t seq_no) {
    // 更新成员变量
    sequence_number_ = seq_no;
    // 使用大端序(网络字节序)写入序列号到第3-4字节
    webrtc::ByteWriter<uint16_t>::WriteBigEndian(WriteAt(2), seq_no);
}

// 设置RTP包的时间戳
void RtpPacket::SetTimestamp(uint32_t ts) {
    // 更新成员变量
    timestamp_ = ts;
    // 使用大端序(网络字节序)写入时间戳到第5-8字节
    webrtc::ByteWriter<uint32_t>::WriteBigEndian(WriteAt(4), ts);
}

// 设置RTP包的SSRC(同步源标识符)
void RtpPacket::SetSsrc(uint32_t ssrc) {
    // 更新成员变量
    ssrc_ = ssrc;
    // 使用大端序(网络字节序)写入SSRC到第9-12字节
    webrtc::ByteWriter<uint32_t>::WriteBigEndian(WriteAt(8), ssrc);
}

// 设置RTP包的负载大小，返回负载数据的起始位置
uint8_t* RtpPacket::SetPayloadSize(size_t bytes_size) {
    // 检查是否有足够的空间存储负载
    if (payload_offset_ + bytes_size > capacity()) {
        // 空间不足，记录警告日志
        RTC_LOG(LS_WARNING) << "set payload size failed, no enough space in buffer";
        return nullptr;
    }

    // 更新负载大小
    payload_size_ = bytes_size;
    // 调整缓冲区大小为头部加负载
    buffer_.SetSize(payload_offset_ + payload_size_);
    // 返回负载数据的起始位置
    return WriteAt(payload_offset_);
}

// 分配负载空间，先清空现有负载，再设置新的负载大小
uint8_t* RtpPacket::AllocatePayload(size_t payload_size) {
    // 先将负载大小设为0，清空现有负载
    SetPayloadSize(0);
    // 设置新的负载大小并返回负载起始位置
    return SetPayloadSize(payload_size);
}

} // namespace xrtc