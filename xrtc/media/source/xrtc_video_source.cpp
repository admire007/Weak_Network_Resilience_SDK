#include "xrtc/media/source/xrtc_video_source.h"

#include <rtc_base/logging.h>

#include "xrtc/media/base/out_pin.h"
namespace xrtc{

XRTCVideoSource::XRTCVideoSource() :
    out_pin_(std::make_unique<OutPin>(this))
{
    //设置MediaFormat为连接匹配做准备
    MediaFormat fmt;//可在此处把宽高设置进去
    fmt.media_type = MainMediaType::kMainTypeVideo;
    fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeI420;
    out_pin_->set_format(fmt);
}

XRTCVideoSource::~XRTCVideoSource() {

}

bool XRTCVideoSource::Start() {
    return true;
}

void XRTCVideoSource::Stop() {
    RTC_LOG(LS_INFO) << "XRTCVideoSource Stop";
}

//将数据抛出到链条上
void XRTCVideoSource::OnFrame(std::shared_ptr<MediaFrame> frame) {
    if (out_pin_) {
        out_pin_->PushMediaFrame(frame);
    }
}

} // namespace xrtc