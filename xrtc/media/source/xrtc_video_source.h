#ifndef XRTCSDK_XRTC_MEDIA_SOURCE_XRTC_VIDEO_SOURCE_H_
#define XRTCSDK_XRTC_MEDIA_SOURCE_XRTC_VIDEO_SOURCE_H_

#include "xrtc/xrtc.h"
#include "xrtc/media/base/media_chain.h"

namespace xrtc {

    class OutPin;

class XRTCVideoSource : public IXRTCConsumer,
                        public MediaObject//所有处理节点的一个基点
{
public:
    XRTCVideoSource();
    ~XRTCVideoSource() override;

    // MediaObject
    bool Start() override;
    void Stop() override;
    std::vector<InPin*> GetAllInPins() override {
        return std::vector<InPin*>();
    }

    std::vector<OutPin*> GetAllOutPins() {
        return std::vector<OutPin*>({ out_pin_.get() });
        //return std::vector<OutPin*>();
    }
   

    // IXRTCConsumer
    void OnFrame(std::shared_ptr<MediaFrame> frame) override;

private:
    std::unique_ptr<OutPin> out_pin_;//video_source 只有输出，没有输入
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_MEDIA_SOURCE_XRTC_VIDEO_SOURCE_H_