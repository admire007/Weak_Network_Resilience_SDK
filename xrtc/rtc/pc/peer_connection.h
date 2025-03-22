#ifndef XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_
#define XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_

#include <string>
#include <memory>

#include "xrtc/rtc/pc/session_description.h"



namespace xrtc {


    class PeerConnection

    {
    public:
        PeerConnection();
        ~PeerConnection();

        int SetRemoteSDP(const std::string& sdp);

    private:
        std::unique_ptr<SessionDescription> remote_desc_;

    };

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_PC_PEER_CONNECTION_H_