#ifndef XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_
#define XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_

#include <string>
#include <memory>

#include "xrtc/rtc/pc/session_description.h"
#include "xrtc/rtc/pc/transport_controller.h"



namespace xrtc {

    struct RTCOfferAnswerOptions {
        bool send_audio = true;
        bool send_video = true;
        bool recv_video = true;
        bool recv_audio = true;
        bool use_rtp_mux = true;
        bool use_rtcp_mux = true;

    };

    class PeerConnection

    {
    public:
        PeerConnection();
        ~PeerConnection();

        int SetRemoteSDP(const std::string& sdp);
        std::string CreateAnswer(const RTCOfferAnswerOptions& options, const std::string& stream_id);

    private:
        std::unique_ptr<SessionDescription> remote_desc_;
        std::unique_ptr<SessionDescription> local_desc_;
        std::unique_ptr<TransportController> transport_controller_;

        uint32_t local_audio_ssrc_ = 0;
        uint32_t local_video_ssrc_ = 0;
        uint32_t local_video_rtx_ssrc_ = 0;
        uint32_t audio_pt_ = 0;
        uint8_t video_pt_ = 0;
        uint8_t video_rtx_pt_ = 0;

    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_