// 防止头文件重复包含的宏定义
#ifndef XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_FORMAT_H264_H_
#define XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_FORMAT_H264_H_

// 标准库头文件
#include <vector> // 用于存储NALU索引
#include <deque>  // 用于存储输入片段
#include <queue>  // 用于存储打包单元

// WebRTC相关头文件
#include <api/array_view.h> // 提供数组视图功能

// 包含基类头文件
#include "xrtc/rtc/modules/rtp_rtcp/rtp_format.h"

namespace xrtc
{

    // NALU索引结构体，用于定位H.264 NALU在缓冲区中的位置
    struct NaluIndex
    {
        size_t start_offset;         // NALU的起始位置，包含起始码
        size_t payload_start_offset; // NALU负载的起始位置（跳过起始码）
        size_t payload_size;         // NALU负载大小
    };

    // H.264 NALU类型枚举，定义了RFC3984中的NAL单元类型
    enum NaluType : uint8_t
    {
        kSlice = 1,  // 非关键帧片段
        kIdr = 5,    // 即时解码刷新帧（关键帧）
        kSei = 6,    // 补充增强信息
        kSps = 7,    // 序列参数集
        kPps = 8,    // 图像参数集
        kStapA = 24, // 单时间聚合包A
        kFuA = 28,   // 分片单元A
    };

    // H.264专用的RTP打包器实现类
    class RtpPacketizerH264 : public RtpPacketizer
    {
    public:
        // 构造函数，接收原始H.264数据和配置参数
        RtpPacketizerH264(rtc::ArrayView<const uint8_t> payload,
                          const RtpPacketizer::Config &config);
        // 析构函数
        ~RtpPacketizerH264() override = default;

        // 实现基类虚函数，返回打包后的包数量
        size_t NumPackets() override;
        // 实现基类虚函数，获取下一个RTP包
        bool NextPacket(RtpPacketToSend *rtp_packet) override;

    private:
        // 打包单元结构体，表示一个RTP包的信息
        struct PacketUnit
        {
            // 构造函数
            PacketUnit(rtc::ArrayView<const uint8_t> source_fragment,
                       bool first_fragment, // 是否是NALU的第一个分片
                       bool last_fragment,  // 是否是NALU的最后一个分片
                       bool aggregated,     // 是否是聚合包
                       uint8_t header) :    // 原始NALU头部
                                         source_fragment(source_fragment),
                                         first_fragment(first_fragment),
                                         last_fragment(last_fragment),
                                         aggregated(aggregated),
                                         header(header)
            {
            }

            rtc::ArrayView<const uint8_t> source_fragment; // 指向源数据的视图
            bool first_fragment;                           // 是否是第一个分片
            bool last_fragment;                            // 是否是最后一个分片
            bool aggregated;                               // 是否是聚合包
            uint8_t header;                                // 原始NALU头部
        };

        // 在缓冲区中查找所有NALU的索引
        std::vector<NaluIndex> FindNaluIndices(const uint8_t *buffer,
                                               size_t buffer_size);
        // 生成所有RTP包
        bool GeneratePackets();
        // FU-A分片打包方法
        bool PacketizeFuA(size_t fragment_index);
        // STAP-A聚合打包方法
        size_t PacketizeStapA(size_t fragment_index);
        // 处理下一个聚合包
        void NextAggregatedPacket(RtpPacketToSend *rtp_packet, const PacketUnit &packet);
        // 处理下一个分片包
        void NextFragmentPacket(RtpPacketToSend *rtp_packet, const PacketUnit &packet);

    private:
        RtpPacketizer::Config config_;                              // 打包配置
        std::deque<rtc::ArrayView<const uint8_t>> input_fragments_; // 输入的NALU片段队列
        std::queue<PacketUnit> packets_;                            // 打包单元队列
        size_t num_packets_left_ = 0;                               // 剩余包数量
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_FORMAT_H264_H_