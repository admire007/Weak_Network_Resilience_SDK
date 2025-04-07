#ifndef XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_
#define XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_

#include <string>
#include <memory>
#include <system_wrappers/include/clock.h>

#include "xrtc/rtc/pc/session_description.h"
#include "xrtc/rtc/pc/transport_controller.h"
#include "xrtc/rtc/pc/peer_connection_def.h"
#include "xrtc/media/base/media_frame.h"
#include <xrtc/rtc/modules/rtp_rtcp/rtp_packet_to_send.h>



namespace xrtc {

    struct RTCOfferAnswerOptions {
        bool send_audio = true;
        bool send_video = true;
        bool recv_video = true;
        bool recv_audio = true;
        bool use_rtp_mux = true;
        bool use_rtcp_mux = true;

    };

    class PeerConnection :public sigslot::has_slots<>

    {
    public:
        PeerConnection();
        ~PeerConnection();

        int SetRemoteSDP(const std::string& sdp);
        std::string CreateAnswer(const RTCOfferAnswerOptions& options, const std::string& stream_id);
        bool SendEncodeImage(std::shared_ptr<MediaFrame> frame);

        sigslot::signal2<PeerConnection*, PeerConnectionState> SignalConnectionState;
        sigslot::signal5<PeerConnection*, int64_t, int32_t, uint8_t, uint32_t>
            SignalNetworkInfo;

    private :
        void OnIceState(TransportController*, ice::IceTransportState ice_state);
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

        // 按照规范该值的初始值需要随机
        uint16_t audio_seq_ = 1000;
        uint16_t video_seq_ = 1000;

        PeerConnectionState pc_state_ = PeerConnectionState::kNew;
        webrtc::Clock* clock_;
        std::vector<std::shared_ptr<RtpPacketToSend>> video_cache_;
    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_