#include "xrtc/rtc/pc/transport_controller.h"

#include "xrtc/base/xrtc_global.h"
#include "xrtc/rtc/pc/session_description.h"

namespace xrtc {

TransportController::TransportController() :
    ice_agent_(new ice::IceAgent(XRTCGlobal::Instance()->network_thread(),
        XRTCGlobal::Instance()->port_allocator()))
{
 
}

TransportController::~TransportController() {
    if (ice_agent_) {
        ice_agent_->Destroy();
        ice_agent_ = nullptr;
    }
}

int TransportController::SetRemoteSDP(SessionDescription* desc) {
    if (!desc) {//检查传入的 SessionDescription 指针是否有效
        return -1;
    }

    /*遍历 SDP 中的所有媒体内容（通常是音频和视频）。
    如果启用了 BUNDLE 功能（允许多个媒体流共用一个传输通道），
    则只处理第一个 BUNDLE 媒体，跳过其他媒体，因为它们会共享同一个传输通道。*/
    for (auto content : desc->contents()) {
        std::string mid = content->mid();
        //开启了bundle功能，audio和video是复用一个通道的
        //只需要第一个bundle去创建即可
        if (desc->IsBundle(mid) && mid != desc->GetFirstBundleId()) {
            continue;
        }

        // 创建ICE transport
        // RTCP, 默认开启a=rtcp:mux
        /*为每个需要的媒体创建 ICE 传输通道。参数 1 表示这是 RTP 通道（而不是 RTCP 通道）。
        由于启用了 RTCP 多路复用（rtcp-mux），所以只需要创建 RTP 通道，RTCP 数据会通过同一个通道传输。*/
        ice_agent_->CreateIceTransport(mid, 1); // 1: RTP

        // 设置ICE param
        /*从 SDP 中获取并设置远端的 ICE 参数（用户名片段和密码）。这些参数用于 ICE 连接过程中的身份验证。*/
        auto td = desc->GetTransportInfo(mid);
        if (td) {
            ice_agent_->SetRemoteIceParams(mid, 1, ice::IceParameters(
                td->ice_ufrag, td->ice_pwd));
        }

        // 设置ICE candidate
        /*将远端提供的所有 ICE 候选者添加到对应的传输通道中，这些候选者代表了可能的连接端点。*/
        for (auto candidate : content->candidates()) {
            ice_agent_->AddRemoteCandidate(mid, 1, candidate);
        }
    }

    return 0;
}



} // namespace xrtc