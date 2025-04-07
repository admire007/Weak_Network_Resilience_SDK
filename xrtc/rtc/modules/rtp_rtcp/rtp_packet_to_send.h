// 防止头文件重复包含的宏定义
#ifndef XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_PACKET_TO_SEND_H_
#define XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_PACKET_TO_SEND_H_

// 包含RTP包基类的头文件
#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet.h"

namespace xrtc {

// RtpPacketToSend类：用于表示要发送的RTP数据包
// 继承自RtpPacket基类，可以使用基类提供的所有功能
class RtpPacketToSend : public RtpPacket {
public:
    // 默认构造函数，创建一个默认容量的RTP发送包
    RtpPacketToSend();
    // 带容量参数的构造函数，创建指定容量的RTP发送包
    RtpPacketToSend(size_t capacity);

};

} // namespace xrtc 结束命名空间

#endif // XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_PACKET_TO_SEND_H_ 结束头文件防重包含宏