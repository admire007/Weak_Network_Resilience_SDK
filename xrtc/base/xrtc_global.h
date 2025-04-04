﻿#ifndef XRTCSDK_XRTC_BASE_XRTC_GLOBAL_H_
#define XRTCSDK_XRTC_BASE_XRTC_GLOBAL_H_

#include <rtc_base/thread.h>
#include <modules/video_capture/video_capture.h>
#include <ice/port_allocator.h>

namespace xrtc {

class XRTCEngineObserver;
class HttpManager;

// 单例模式
class XRTCGlobal {
public:
    static XRTCGlobal* Instance();

    XRTCEngineObserver* engine_observer() { return engine_observer_; }
    void RegisterEngineObserver(XRTCEngineObserver* observer) {
        engine_observer_ = observer;

    }


    rtc::Thread* api_thread() { return api_thread_.get(); }
    rtc::Thread* worker_thread() { return worker_thread_.get(); }
    rtc::Thread* network_thread() { return network_thread_.get(); }
    
    webrtc::VideoCaptureModule::DeviceInfo* video_device_info() {
        return video_device_info_.get();
    }

private:
    XRTCGlobal();
    ~XRTCGlobal();

private:
    std::unique_ptr<rtc::Thread> api_thread_;
    std::unique_ptr<rtc::Thread> worker_thread_;
    std::unique_ptr<rtc::Thread> network_thread_;
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> video_device_info_;
    XRTCEngineObserver* engine_observer_ = nullptr;
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_BASE_XRTC_GLOBAL_H_