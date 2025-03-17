#ifndef XRTCSDK_XRTC_MEDIA_BASE_MEDIA_FRAME_H_
#define XRTCSDK_XRTC_MEDIA_BASE_MEDIA_FRAME_H_

namespace xrtc {

    //定义媒体的主要类型
enum class MainMediaType {
    kMainTypeCommon,
    kMainTypeAudio,
    kMainTypeVideo,
    kMainTypeData
};

//定义媒体的子类型，用于更细致地区分媒体格式。
enum class SubMediaType {
    kSubTypeCommon,
    kSubTypeI420,
    kSubTypeH264,
};

//描述音频格式的具体信息。
struct AudioFormat {
    SubMediaType type;
};

//描述视频格式的具体信息。
struct VideoFormat {
    SubMediaType type;
    int width;
    int height;
    bool idr;//是否是关键帧
};

//统一描述媒体格式，支持音频和视频两种类型
class MediaFormat {
public:
    MainMediaType media_type;
    union {
        AudioFormat audio_fmt;
        VideoFormat video_fmt;
    } sub_fmt;
};

class MediaFrame {
public:
    MediaFrame(int size) : max_size(size) {
        memset(data, 0, sizeof(data));
        memset(data_len, 0, sizeof(data_len));
        memset(stride, 0, sizeof(stride));
        data[0] = new char[size];
        data_len[0] = size;
    }

    ~MediaFrame() {
        if (data[0]) {
            delete[] data[0];
            data[0] = nullptr;
        }
    }
    
public:
    int max_size;
    MediaFormat fmt;
    char* data[4];//4个平面
    int data_len[4];//指定每个平面字节数
    int stride[4];//存放每一行的一个大小
    uint32_t ts = 0;//帧的时间戳
    int64_t capture_time_ms = 0;
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_MEDIA_BASE_MEDIA_FRAME_H_

