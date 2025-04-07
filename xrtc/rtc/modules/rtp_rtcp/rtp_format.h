// 防止头文件重复包含的宏定义
#ifndef XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_FORMAT_H_
#define XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_FORMAT_H_

// 标准库头文件
#include <memory>  // 提供智能指针相关功能
#include <vector>  // 提供vector容器

// WebRTC相关头文件
#include <api/video/video_codec_type.h>  // 视频编码类型定义
#include <api/array_view.h>  // 提供数组视图功能，无需拷贝数据

namespace xrtc {

// 前向声明RtpPacketToSend类
class RtpPacketToSend;

// RTP打包器基类，定义了打包接口和通用方法
class RtpPacketizer {
public:
    // 负载限制结构体，用于控制RTP包的大小
    struct PayloadLimits {
        int max_payload_len = 1200;  // 最大负载长度，默认1200字节
        int single_packet_reduction_len = 0;  // 单包模式下的长度减少值
        int first_packet_reduction_len = 0;   // 第一个包的长度减少值
        int last_packet_reduction_len = 0;    // 最后一个包的长度减少值
    };

    // 配置结构体，包含负载限制信息
    struct Config {
        PayloadLimits limits;  // 负载限制配置
    };

    // 工厂方法，根据编码类型创建对应的打包器实例
    static std::unique_ptr<RtpPacketizer> Create(webrtc::VideoCodecType type,
        rtc::ArrayView<const uint8_t> payload,
        const RtpPacketizer::Config& config);

    // 将负载均匀分割成多个片段，用于分片打包
    std::vector<int> SplitAboutEqual(size_t payload_size, const PayloadLimits& limits);

    // 虚析构函数
    virtual ~RtpPacketizer() = default;
    // 获取打包后的包数量
    virtual size_t NumPackets() = 0;
    // 获取下一个RTP包
    virtual bool NextPacket(RtpPacketToSend* packet) = 0;
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_FORMAT_H_