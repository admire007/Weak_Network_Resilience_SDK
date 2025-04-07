// 防止头文件重复包含的宏定义
#ifndef XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_PACKET_H_
#define XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_PACKET_H_

// 包含WebRTC基础库中的写时复制缓冲区，用于高效管理RTP包数据
#include <rtc_base/copy_on_write_buffer.h>

// XRTC SDK的命名空间
namespace xrtc {

// RTP包类，用于创建、管理和操作RTP协议数据包
class RtpPacket {
public:
    // 默认构造函数，使用默认容量创建RTP包
    RtpPacket();
    // 带指定容量的构造函数
    RtpPacket(size_t capacity);

    // 获取RTP包的序列号，用于包的排序和丢包检测
    uint16_t sequence_number() const { return sequence_number_; }
    // 获取标记位，通常用于标识帧边界（如视频的最后一个分片）
    bool marker() const { return marker_; }
    // 获取时间戳，表示媒体采样的时间点
    uint32_t timestamp() const { return timestamp_; }
    // 获取负载数据的只读视图，不需要复制数据
    rtc::ArrayView<const uint8_t> payload() const {
        return rtc::MakeArrayView(data() + payload_offset_, payload_size_);
    }

    // 获取RTP头部大小（字节数）
    size_t header_size() const { return payload_offset_; }
    // 获取负载数据大小（字节数）
    size_t payload_size() const { return payload_size_; }
    // 获取填充数据大小（字节数）
    size_t padding_size() const { return padding_size_; }

    // 获取整个RTP包数据的只读指针
    const uint8_t* data() const { return buffer_.cdata(); }
    // 获取整个RTP包的总大小（头部+负载+填充）
    size_t size() {
        return payload_offset_ + payload_size_ + padding_size_;
    }
    // 获取缓冲区的总容量
    size_t capacity() { return buffer_.capacity(); }
    // 获取缓冲区的剩余可用容量
    size_t FreeCapacity() { return capacity() - size(); }
    // 清空RTP包，重置所有字段
    void Clear();

    // 设置标记位
    void SetMarker(bool marker_bit);
    // 设置负载类型，标识媒体类型（如音频、视频）和编码格式
    void SetPayloadType(uint8_t payload_type);
    // 设置序列号
    void SetSequenceNumber(uint16_t seq_no);
    // 设置时间戳
    void SetTimestamp(uint32_t ts);
    // 设置同步源标识符(SSRC)，用于标识RTP流的发送者
    void SetSsrc(uint32_t ssrc);
    // 设置负载大小，返回负载数据的起始位置
    uint8_t* SetPayloadSize(size_t bytes_size);

    // 分配负载空间，返回负载数据的起始位置
    uint8_t* AllocatePayload(size_t payload_size);

    // 获取指定偏移位置的可写指针，用于直接写入数据
    uint8_t* WriteAt(size_t offset) {
        return buffer_.MutableData() + offset;
    }

    // 在指定偏移位置写入一个字节
    void WriteAt(size_t offset, uint8_t byte) {
        buffer_.MutableData()[offset] = byte;
    }

private:
    // 标记位，表示特殊帧边界
    bool marker_;
    // 负载类型，标识媒体类型和编码格式
    uint8_t payload_type_;
    // 序列号，用于包的排序和丢包检测
    uint16_t sequence_number_;
    // 时间戳，表示媒体采样的时间点
    uint32_t timestamp_;
    // 同步源标识符，标识RTP流的发送者
    uint32_t ssrc_;
    // 负载数据在缓冲区中的起始偏移位置
    size_t payload_offset_;
    // 负载数据的大小
    size_t payload_size_;
    // 填充数据的大小
    size_t padding_size_;
    // 存储RTP包数据的写时复制缓冲区
    rtc::CopyOnWriteBuffer buffer_;
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_PACKET_H_