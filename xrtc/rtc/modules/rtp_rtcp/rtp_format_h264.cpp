// 包含H.264 RTP打包器的头文件
#include "xrtc/rtc/modules/rtp_rtcp/rtp_format_h264.h"

// 包含字节操作工具
#include <modules/rtp_rtcp/source/byte_io.h>

// 包含RTP包发送类的头文件
#include "xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h"

namespace xrtc {

// 定义常量：NALU起始码大小（3字节）
const size_t kNaluShortStartSequenceSize = 3;
// 定义常量：FU-A头部大小（2字节：FU indicator + FU header）
const size_t kFuAHeaderSize = 2;
// 定义常量：NALU头部大小（1字节）
const size_t kNaluHeaderSize = 1;
// 定义常量：STAP-A中长度字段大小（2字节）
const size_t kLengthFieldSize = 2;

// NAL头部的bit mask定义
enum NalDef : uint8_t {
    kFBit = 0x80,    // 禁止位（第0位，1表示语法错误）
    kNriMask = 0x60, // NRI重要性位（第1-2位）
    kTypeMask = 0x1F,// 类型位（第3-7位）
};

// FU-Header bit mask定义
enum FuDef : uint8_t {
    kSBit = 0x80,    // 起始位（第0位，1表示分片的第一个包）
    kEBit = 0x40,    // 结束位（第1位，1表示分片的最后一个包）
    kRBit = 0x20,    // 保留位（第2位，必须为0）
};

// H.264 RTP打包器构造函数
RtpPacketizerH264::RtpPacketizerH264(
    rtc::ArrayView<const uint8_t> payload,
    const RtpPacketizer::Config& config) :
    config_(config)
{
    // 查找所有NALU并存储到输入片段队列
    for (const auto& nalu : FindNaluIndices(payload.data(), payload.size())) {
        input_fragments_.push_back(payload.subview(nalu.payload_start_offset, nalu.payload_size));
    }

    // 生成所有RTP包，如果失败则清空队列
    if (!GeneratePackets()) {
        num_packets_left_ = 0;
        while (!packets_.empty()) {
            packets_.pop();
        }
    }
}

// 获取打包后的包数量
size_t RtpPacketizerH264::NumPackets() {
    return num_packets_left_;
}

// 获取下一个RTP包
bool RtpPacketizerH264::NextPacket(RtpPacketToSend* rtp_packet) {
    // 如果没有包可发送，返回false
    if (packets_.empty()) {
        return false;
    }

    // 获取队列前端的包
    //PacketUnit* packet = &packets_.front();
    //if (packet->first_fragment && packet->last_fragment) {
    //    // 单个NALU包：直接复制整个NALU
    //    size_t packet_size = packet->source_fragment.size();
    //    uint8_t* buffer = rtp_packet->AllocatePayload(packet_size);
    //    memcpy(buffer, packet->source_fragment.data(), packet_size);
    //    packets_.pop();
    //    input_fragments_.pop_front();
    //}
    //else if (packet->aggregated) {
    //    // STAP-A聚合包：调用聚合包处理函数
    //    NextAggregatedPacket(rtp_packet);
    //}
    //else {
    //    // FU-A分片包：调用分片包处理函数
    //    NextFragmentPacket(rtp_packet);
    //}

    //// 更新剩余包数量
    //--num_packets_left_;
    //// 设置RTP包的标记位（最后一个包标记为true）
    //rtp_packet->SetMarker(packets_.empty());

    return true;
}

// 在缓冲区中查找所有NALU的索引
std::vector<NaluIndex> RtpPacketizerH264::FindNaluIndices(const uint8_t* buffer, 
    size_t buffer_size) 
{
    // 存储找到的所有NALU索引
    std::vector<NaluIndex> sequences;
    // 如果缓冲区太小，无法包含最小的起始码，则返回空结果
    if (buffer_size < kNaluShortStartSequenceSize) {
        return sequences;
    }

    // 计算搜索的结束位置
    size_t end = buffer_size - kNaluShortStartSequenceSize;
    for (size_t i = 0; i < end; ) {
        // 查找NALU的起始码模式：0x000001
        if (buffer[i + 2] > 1) {
            // 如果第三个字节>1，则不可能是起始码，跳过3个字节
            i += 3;
        }
        else if (buffer[i + 2] == 1) {
            // 如果第三个字节=1，检查前两个字节是否为0
            if (buffer[i] == 0 && buffer[i + 1] == 0) {
                // 找到了一个起始码（至少3字节：0x000001）
                NaluIndex index = { i, i + 3, 0 };
                // 检查是否是4字节的起始码（0x00000001）
                if (index.start_offset > 0 && buffer[index.start_offset - 1] == 0) {
                    --index.start_offset;
                }

                // 如果已有NALU，计算其负载大小（到当前NALU起始位置）
                auto it = sequences.rbegin();
                if (it != sequences.rend()) {
                    it->payload_size = index.start_offset - it->payload_start_offset;
                }

                // 添加新找到的NALU索引
                sequences.push_back(index);
            }

            // 跳过3个字节继续搜索
            i += 3;
        }
        else if (buffer[i + 2] == 0) {
            // 如果连续遇到0，只前进1字节，以便检测可能的起始码
            i += 1;
        }
    }

    // 计算最后一个NALU的负载大小（到缓冲区结束）
    auto it = sequences.rbegin();
    if (it != sequences.rend()) {
        it->payload_size = buffer_size - it->payload_start_offset;
    }

    return sequences;
}

// 生成所有RTP包
bool RtpPacketizerH264::GeneratePackets() {
    // 遍历所有NALU片段
    for (size_t i = 0; i < input_fragments_.size(); ) {
        size_t fragment_len = input_fragments_[i].size();
        // 计算当前NALU的最大负载容量
        size_t single_packet_capacity = config_.limits.max_payload_len;
        if (input_fragments_.size() == 1) {
            // 如果只有一个NALU，应用单包减少长度
            single_packet_capacity -= config_.limits.single_packet_reduction_len;
        }
        else if (i == 0) { // 第一个NALU
            single_packet_capacity -= config_.limits.first_packet_reduction_len;
        }
        else if (i + 1 == input_fragments_.size()) { // 最后一个NALU
            single_packet_capacity -= config_.limits.last_packet_reduction_len;
        }

        if (fragment_len > single_packet_capacity) { 
            // 如果NALU大于单包容量，使用FU-A分片打包
            if (!PacketizeFuA(i)) {
                return false;
            }
            ++i; // 处理下一个NALU
        }
        else {
            // 如果NALU小于单包容量，尝试使用STAP-A聚合打包
            i = PacketizeStapA(i); // 返回下一个待处理的NALU索引
        }
    }
    return true;
}

// FU-A分片打包方法
bool RtpPacketizerH264::PacketizeFuA(size_t fragment_index) {
    // 获取当前NALU片段
    rtc::ArrayView<const uint8_t> fragment = input_fragments_[fragment_index];
    PayloadLimits limits = config_.limits;
    // 预留FU-A头部的空间（2字节）
    limits.max_payload_len -= kFuAHeaderSize;
    
    // 根据NALU在序列中的位置调整限制参数
    if (input_fragments_.size() != 1) {
        if (fragment_index == input_fragments_.size() - 1) {
            // 最后一个NALU：包含中间包和最后一个包
            limits.single_packet_reduction_len = limits.last_packet_reduction_len;
        }
        else if (fragment_index == 0) {
            // 第一个NALU：包含第一个包和中间包
            limits.single_packet_reduction_len = limits.first_packet_reduction_len;
        }
        else {
            // 中间NALU：只包含中间包
            limits.single_packet_reduction_len = 0;
        }
    }

    // 如果不是第一个NALU，则不应用第一个包的减少长度
    if (fragment_index != 0) {
        limits.first_packet_reduction_len = 0;
    }

    // 如果不是最后一个NALU，则不应用最后一个包的减少长度
    if (fragment_index != input_fragments_.size() - 1) {
        limits.last_packet_reduction_len = 0;
    }

    // 计算需要分片的负载大小（不包括NALU头部）
    size_t payload_left = fragment.size() - kNaluHeaderSize;
    // 负载的起始偏移量（跳过NALU头部）
    size_t offset = kNaluHeaderSize;
    // 将负载大小分割成大体相同的几个部分
    std::vector<int> payload_sizes = SplitAboutEqual(payload_left, limits);
    if (payload_sizes.empty()) {
        return false;
    }

    // 为每个分片创建PacketUnit
    for (size_t i = 0; i < payload_sizes.size(); ++i) {
        size_t packet_length = payload_sizes[i];
        packets_.push(PacketUnit(
            fragment.subview(offset, packet_length),
            i == 0,                              // 是否是第一个分片
            i == payload_sizes.size() - 1,       // 是否是最后一个分片
            false,                               // 不是聚合包
            fragment[0]                          // 保存原始NALU头部
        ));
        
        // 更新偏移量
        offset += packet_length;
    }

    // 更新总包数
    num_packets_left_ += payload_sizes.size();
    
    return true;
}

// STAP-A聚合打包方法
size_t RtpPacketizerH264::PacketizeStapA(size_t fragment_index) {
    // 计算可用的负载空间
    size_t payload_size_left = config_.limits.max_payload_len;
    if (input_fragments_.size() == 1) {
        payload_size_left -= config_.limits.single_packet_reduction_len;
    }
    else if (fragment_index == 0) {
        // 第一个NALU
        payload_size_left -= config_.limits.first_packet_reduction_len;
    }

    // 初始化聚合包计数和头部长度
    int aggregated_fragment = 0;
    int fragment_header_length = 0;
    rtc::ArrayView<const uint8_t> fragment = input_fragments_[fragment_index];
    ++num_packets_left_; // 增加一个包计数

    // 定义lambda函数计算所需的负载大小
    auto payload_size_needed = [&] {
        size_t fragment_size = fragment.size() + fragment_header_length;
        if (input_fragments_.size() == 1) {
            return fragment_size;
        }

        if (fragment_index == input_fragments_.size() - 1) {
            // 如果是最后一个NALU，考虑最后一个包的减少长度
            return fragment_size + config_.limits.last_packet_reduction_len;
        }

        return fragment_size;
    };

    // 尝试聚合多个NALU到一个STAP-A包中
    while (payload_size_left >= payload_size_needed()) {
        // 创建PacketUnit并加入队列
        packets_.push(PacketUnit(
            fragment,
            aggregated_fragment == 0,  // 第一个聚合的NALU
            false,                     // 不是最后一个分片（后面会更新）
            true,                      // 是聚合包
            fragment[0]                // 保存原始NALU头部
        ));
        // 更新剩余空间
        payload_size_left -= fragment.size();
        payload_size_left -= fragment_header_length;

        // 更新下一个NALU的头部长度
        fragment_header_length = kLengthFieldSize; // 每个NALU需要2字节长度字段
        if (0 == aggregated_fragment) {
            // 第一个NALU还需要STAP-A头部(1字节)和第一个长度字段(2字节)
            fragment_header_length += (kNaluHeaderSize + kLengthFieldSize);
        }

        ++aggregated_fragment; // 增加聚合计数

        // 尝试聚合下一个NALU
        ++fragment_index;
        if (fragment_index == input_fragments_.size()) {
            break; // 没有更多NALU可聚合
        }

        // 获取下一个NALU
        fragment = input_fragments_[fragment_index];
    }

    // 标记最后一个聚合的NALU为last_fragment
    packets_.back().last_fragment = true;

    // 返回下一个待处理的NALU索引
    return fragment_index;
}

// 处理下一个聚合包（STAP-A）
void RtpPacketizerH264::NextAggregatedPacket(RtpPacketToSend* rtp_packet) {
    // 分配RTP包负载空间
    size_t rtp_packet_capacity = rtp_packet->FreeCapacity();
    uint8_t* buffer = rtp_packet->AllocatePayload(rtp_packet_capacity);
    PacketUnit* packet = &packets_.front();
    
    // 写入STAP-A头部（保留原NALU的F和NRI位，类型改为STAP-A）
    buffer[0] = (packet->header & (kFBit | kNriMask)) | NaluType::kStapA;
    size_t index = kNaluHeaderSize;
    bool is_last_fragment = packet->last_fragment;
    
    // 写入所有聚合的NALU
    while (packet->aggregated) {
        rtc::ArrayView<const uint8_t> fragment = packet->source_fragment;
        // 写入NALU长度字段（2字节，大端序）
        webrtc::ByteWriter<uint16_t>::WriteBigEndian(&buffer[index], (uint16_t)fragment.size());
        index += kLengthFieldSize;
        // 写入NALU数据
        memcpy(&buffer[index], fragment.data(), fragment.size());
        index += fragment.size();
        // 移除已处理的包
        packets_.pop();
        input_fragments_.pop_front();
        
        // 如果是最后一个聚合的NALU，退出循环
        if (is_last_fragment) {
            break;
        }

        // 获取下一个包
        packet = &packets_.front();
        is_last_fragment = packet->last_fragment;
    }

    // 设置RTP包的实际负载大小
    rtp_packet->SetPayloadSize(index);
}

// 处理下一个分片包（FU-A）
void RtpPacketizerH264::NextFragmentPacket(RtpPacketToSend* rtp_packet) {
    PacketUnit* packet = &packets_.front();
    
    // 构造FU-Indicator（保留原NALU的F和NRI位，类型改为FU-A）
    uint8_t fu_indicator = (packet->header & (kFBit | kNriMask)) |
        NaluType::kFuA;
        
    // 构造FU-Header
    uint8_t fu_header = 0;
    fu_header |= (packet->first_fragment ? kSBit : 0); // 起始位
    fu_header |= (packet->last_fragment ? kEBit : 0);  // 结束位
    // 提取原始NALU类型（低5位）
    uint8_t type = packet->header & kTypeMask;
    fu_header |= type; // 设置NALU类型
    
    // 获取分片数据
    rtc::ArrayView<const uint8_t> fragment = packet->source_fragment;
    // 分配RTP包负载空间（FU-A头部 + 分片数据）
    uint8_t* buffer = rtp_packet->AllocatePayload(kFuAHeaderSize + fragment.size());
    // 写入FU-A头部
    buffer[0] = fu_indicator;
    buffer[1] = fu_header;
    // 写入分片数据
    memcpy(buffer + kFuAHeaderSize, fragment.data(), fragment.size());
    
    // 移除已处理的包
    packets_.pop();
    // 如果是最后一个分片，移除对应的输入片段
    if (packet->last_fragment) {
        input_fragments_.pop_front();
    }
}

} // namespace xrtc