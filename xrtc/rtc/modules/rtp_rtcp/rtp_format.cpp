// 包含RTP打包器的头文件
#include "xrtc/rtc/modules/rtp_rtcp/rtp_format.h"

// 包含H.264特定的RTP打包器实现
#include "xrtc/rtc/modules/rtp_rtcp/rtp_format_h264.h"

namespace xrtc {

// 工厂方法实现，根据视频编码类型创建对应的打包器实例
std::unique_ptr<RtpPacketizer> RtpPacketizer::Create(webrtc::VideoCodecType type, 
    rtc::ArrayView<const uint8_t> payload,
    const RtpPacketizer::Config& config) 
{
    // 根据编码类型选择合适的打包器
    switch (type) {
    case webrtc::kVideoCodecH264:
        // 创建H.264专用的RTP打包器
        return std::make_unique<RtpPacketizerH264>(payload, config);
    default:
        // 不支持的编码类型返回nullptr
        return nullptr;
    }
}

// 将负载均匀分割成多个片段的方法实现
std::vector<int> RtpPacketizer::SplitAboutEqual(size_t payload_size, 
    const PayloadLimits& limits) 
{
    std::vector<int> result;
    // 如果容量足够放入单个包，则不需要分片
    if (limits.max_payload_len >= payload_size + limits.single_packet_reduction_len) {
        result.push_back(payload_size);
        return result;
    }

    // 如果容量太小，无法容纳最小分片，则返回空结果
    if (limits.max_payload_len - limits.first_packet_reduction_len < 1 ||
        limits.max_payload_len - limits.last_packet_reduction_len < 1) 
    {
        return result;
    }

    // 计算需要均分的总字节数（包括额外开销）
    size_t total_bytes = payload_size + limits.first_packet_reduction_len
        + limits.last_packet_reduction_len;
    // 计算需要分配的包数量，向上取整
    size_t num_packets_left = (total_bytes + limits.max_payload_len - 1) /
        limits.max_payload_len;
    // 确保至少分成两个包
    if (num_packets_left == 1) {
        num_packets_left = 2;
    }

    // 计算每个包平均分配的字节数
    size_t bytes_per_packet = total_bytes / num_packets_left;
    // 计算需要多分配1字节的包数量（处理不能整除的情况）
    size_t num_larger_packet = total_bytes % num_packets_left;

    int remain_data = payload_size;
    bool first_packet = true;
    // 循环分配每个包的大小
    while (remain_data > 0) {
        // 如果当前包需要多分配一个字节
        // 示例说明：
        // total_bytes 5
        // 分配的个数3个包
        // 5 / 3 = 1, 5 % 3 = 2，所以有2个包大小为2，1个包大小为1
        if (num_packets_left == num_larger_packet) {
            ++bytes_per_packet;
        }
        int current_packet_bytes = bytes_per_packet;

        // 处理第一个包的特殊情况（考虑first_packet_reduction_len）
        if (first_packet) {
            if (current_packet_bytes - limits.first_packet_reduction_len > 1) {
                current_packet_bytes -= limits.first_packet_reduction_len;
            }
            else {
                // 确保至少有1字节
                current_packet_bytes = 1;
            }
        }

        // 确保不会分配超过剩余数据量
        if (current_packet_bytes > remain_data) {
            current_packet_bytes = remain_data;
        }

        // 确保最后一个包至少能分到1字节数据
        if (num_packets_left == 2 && current_packet_bytes == remain_data) {
            --current_packet_bytes;
        }

        // 更新剩余数据和包数量
        remain_data -= current_packet_bytes;
        num_packets_left--;
        // 将当前包大小添加到结果中
        result.push_back(current_packet_bytes);
        first_packet = false;
    }

    return result;
}

} // namespace xrtc