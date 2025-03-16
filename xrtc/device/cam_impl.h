#ifndef XRTCSDK_XRTC_DEVICE_CAM_IMPL_H_
#define XRTCSDK_XRTC_DEVICE_CAM_IMPL_H_

#include <atomic>
#include <vector>

#include <rtc_base/thread.h>
#include <modules/video_capture/video_capture.h>

#include "xrtc/xrtc.h"

namespace xrtc {

class CamImpl : public IVideoSource,
    public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:
    void Start() override;
    void Stop() override;
    void Destroy() override;
    void Setup(const std::string& json_config);
    //添加/移除消费者  （处理已经获取到的视频数据）
    void AddConsumer(IXRTCConsumer* consumer) override;
    void RemoveConsumer(IXRTCConsumer* consumer) override;

    //VideoSinkInterface  函数库的纯虚函数重写
    void OnFrame(const webrtc::VideoFrame& frame) override;


private:
    CamImpl(const std::string& cam_id);
    ~CamImpl();

    friend class XRTCEngine;//定义友元访问私有

private:
    std::string cam_id_;
    rtc::Thread* current_thread_;//专门的线程启动
    bool has_start_ = false;
    rtc::scoped_refptr<webrtc::VideoCaptureModule> video_capture_;
    webrtc::VideoCaptureModule::DeviceInfo* device_info_;
    std::atomic<int> fps_{0};
    std::atomic<int64_t> last_frame_ts_{0};
    std::atomic<int64_t> start_time_{ 0 };
    std::vector<IXRTCConsumer*> consumer_list_;//可能不只有一个consumer
};

} // namespace xrtc


#endif // XRTCSDK_XRTC_DEVICE_CAM_IMPL_H_