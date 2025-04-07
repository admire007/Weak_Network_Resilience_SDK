// 包含RtpPacketToSend类的头文件定义
#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h"

namespace xrtc {

// 默认构造函数实现
// 调用基类RtpPacket的默认构造函数初始化
// 基类构造函数会设置默认容量(1500字节)并初始化RTP头部
RtpPacketToSend::RtpPacketToSend() :
    RtpPacket()
{
}

// 带容量参数的构造函数实现
// 调用基类RtpPacket的带参构造函数，传入指定的容量大小
// 基类构造函数会分配指定大小的缓冲区并初始化RTP头部
RtpPacketToSend::RtpPacketToSend(size_t capacity) :
    RtpPacket(capacity)
{
}

} // namespace xrtc 结束命名空间