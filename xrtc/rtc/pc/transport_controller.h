#ifndef XRTCSDK_XRTC_RTC_PC_TRANSPORT_CONTROLLER_H_
#define XRTCSDK_XRTC_RTC_PC_TRANSPORT_CONTROLLER_H_

//ice通道管理类
#include <ice/ice_agent.h>

namespace xrtc {

class SessionDescription;


class TransportController {
public:
    TransportController();
    ~TransportController();

    int SetRemoteSDP(SessionDescription* desc);
    
    
  
  

private:
    ice::IceAgent* ice_agent_;
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_PC_TRANSPORT_CONTROLLER_H_