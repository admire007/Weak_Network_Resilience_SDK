﻿#ifndef XRTCSDK_XRTC_MEDIA_BASE_BASE_PIN_H_
#define XRTCSDK_XRTC_MEDIA_BASE_BASE_PIN_H_

#include <memory>
#include "xrtc/media/base/media_frame.h"

namespace xrtc {

class MediaObject;

class BasePin {
public:
    BasePin(MediaObject* obj) : obj_(obj) {}
    virtual ~BasePin() {}

    MediaObject* GetMediaObject() { return obj_; }
    void set_format(const MediaFormat& fmt) { fmt_ = fmt; }
    MediaFormat format() { return fmt_; }//区分视频或音频，相匹配的才能连接在一起
 
    //传递数据，，流动
    virtual void PushMediaFrame(std::shared_ptr<MediaFrame> frame) = 0;

protected:  
    MediaObject* obj_;
    MediaFormat fmt_;
};

} // namespace xrtc


#endif // XRTCSDK_XRTC_MEDIA_BASE_BASE_PIN_H_