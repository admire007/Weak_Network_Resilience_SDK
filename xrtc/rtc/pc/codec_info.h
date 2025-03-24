#ifndef XRTCSDK_XRTC_RTC_PC_CODEC_INFO_H_
#define XRTCSDK_XRTC_RTC_PC_CODEC_INFO_H_

#include <string>
#include <map>
#include <vector>

namespace xrtc {

    // 前向声明音频和视频编解码器信息类
    class AudioCodecInfo;
    class VideoCodecInfo;

    /**
     * @brief 反馈参数类，用于描述编解码器的反馈机制
     * 例如：RTCP反馈如NACK、PLI等
     */
    class FeedbackParam {
    public:
        // 构造函数，创建带参数的反馈
        // @param id 反馈机制的标识符，如"nack"、"transport-cc"等
        // @param param 反馈机制的附加参数
        FeedbackParam(const std::string& id, const std::string& param) :
            id_(id), param_(param) { }

        // 构造函数，创建不带参数的反馈
        // @param id 反馈机制的标识符
        FeedbackParam(const std::string& id) : id_(id), param_("") {}

        // 获取反馈机制的标识符
        std::string id() { return id_; }

        // 获取反馈机制的附加参数
        std::string param() { return param_; }

    private:
        std::string id_;    // 反馈机制的标识符
        std::string param_; // 反馈机制的附加参数
    };

    // 编解码器参数类型定义，键值对形式存储编解码器的附加参数
    // 例如：对于opus可能有{"minptime", "10"}, {"useinbandfec", "1"}
    typedef std::map<std::string, std::string> CodecParam;

    /**
     * @brief 编解码器信息基类，包含编解码器的基本信息
     */
    class CodecInfo {
    public:
        // 尝试将当前对象转换为AudioCodecInfo，如果不是音频编解码器则返回nullptr
        virtual AudioCodecInfo* AsAudio() { return nullptr; }

        // 尝试将当前对象转换为VideoCodecInfo，如果不是视频编解码器则返回nullptr
        virtual VideoCodecInfo* AsVideo() { return nullptr; }

        int id;                                 // 编解码器的负载类型(PT)，如opus通常为111
        std::string name;                       // 编解码器名称，如"opus"、"VP8"、"H264"等
        int clockrate;                          // 时钟频率，单位Hz，如opus为48000
        std::vector<FeedbackParam> feedback_param; // 支持的反馈机制列表
        CodecParam codec_param;                 // 编解码器特定参数，键值对形式
    };

    /**
     * @brief 音频编解码器信息类，继承自CodecInfo
     */
    class AudioCodecInfo : public CodecInfo {
    public:
        // 重写AsAudio方法，返回自身指针
        AudioCodecInfo* AsAudio() override { return this; }

        int channels;  // 音频通道数，如单声道为1，立体声为2
    };

    /**
     * @brief 视频编解码器信息类，继承自CodecInfo
     */
    class VideoCodecInfo : public CodecInfo {
    public:
        // 重写AsVideo方法，返回自身指针
        VideoCodecInfo* AsVideo() override { return this; }

        // 视频编解码器特有的参数可以在这里添加
        // 例如：分辨率、帧率限制等
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_PC_CODEC_INFO_H_