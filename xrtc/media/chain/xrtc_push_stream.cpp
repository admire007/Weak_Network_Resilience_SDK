﻿#include "xrtc/media/chain/xrtc_push_stream.h"

#include <rtc_base/logging.h>

#include "xrtc/base/xrtc_json.h"
#include "xrtc/media/base/xrtc_pusher.h"
#include "xrtc/base/xrtc_global.h"

namespace xrtc {

XRTCPushStream::XRTCPushStream(XRTCPusher* pusher, IVideoSource* video_source) :
    pusher_(pusher),
    //audio_source_(audio_source),
    video_source_(video_source),
    //xrtc_audio_source_(std::make_unique<XRTCAudioSource>()),
    xrtc_video_source_(std::make_unique<XRTCVideoSource>()),
    //audio_processing_filter_(std::make_unique<AudioProcessingFilter>()),
    //opus_encoder_filter_(std::make_unique<OpusEncoderFilter>()),
    x264_encoder_filter_(std::make_unique<X264EncoderFilter>())
    //,xrtc_media_sink_(std::make_unique<XRTCMediaSink>(this))
{
}

XRTCPushStream::~XRTCPushStream() {

}

void XRTCPushStream::Start() {
    RTC_LOG(LS_INFO) << "XRTCPushStream Start";

    XRTCError err = XRTCError::kNoErr;

    do {
        if (!video_source_) {
            err = XRTCError::kPushNoVideoSourceErr;
            RTC_LOG(LS_WARNING) << "PushStream start failed: no video source";
            break;
        }


        if (video_source_) {
            video_source_->AddConsumer(xrtc_video_source_.get());
            AddMediaObject(xrtc_video_source_.get());
            AddMediaObject(x264_encoder_filter_.get());
        }

        if (!ConnectMediaObject(xrtc_video_source_.get(), x264_encoder_filter_.get())) {
            err = XRTCError::kChainConnectErr;
            RTC_LOG(LS_WARNING) << "xrtc_video_source connect to x264_encoder_filter failed";
            break;
        }

        if (!StartChain()) {
            err = XRTCError::kChainStartErr;
            RTC_LOG(LS_WARNING) << "PushStream StartChain failed";
            break;
        }

    } while (false);

    if (err != XRTCError::kNoErr) {
        if (XRTCGlobal::Instance()->engine_observer()) {
            XRTCGlobal::Instance()->engine_observer()->OnPushFailed(pusher_, err);
        }
    }
}

void XRTCPushStream::Stop() {
    // TODO：如果我们先停止设备，在停止推流，此处会crash
    /*if (audio_source_) {
        audio_source_->RemoveConsumer(xrtc_audio_source_.get());
    }

    if (video_source_) {
        video_source_->RemoveConsumer(xrtc_video_source_.get());
    }*/

    StopChain();
}

void XRTCPushStream::Destroy() {
}



} // namespace xrtc