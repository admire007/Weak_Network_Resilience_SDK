#include "xrtc/rtc/pc/session_description.h"

#include <sstream>

namespace xrtc {

SessionDescription::SessionDescription(SdpType type) :
    sdp_type_(type)
{
}

SessionDescription::~SessionDescription() {
}

std::shared_ptr<TransportDescription> SessionDescription::GetTransportInfo(const std::string transport_name)
{
    for (auto td : transport_info_) {
        if (td->mid == transport_name) {
            return td;
        }
    }
    return nullptr;
}

void SessionDescription::AddTransportInfo(std::shared_ptr<TransportDescription> td)
{
    transport_info_.push_back(td);
}

void SessionDescription::AddTransportInfo(const std::string& mid, const ice::IceParameters& ice_param)
{
    auto td = std::make_shared<TransportDescription>();
    td->mid = mid;
    td->ice_ufrag = ice_param.ice_ufrag;
    td->ice_pwd = ice_param.ice_pwd;

    transport_info_.push_back(td);
}


void ContentGroup::AddContentName(const std::string& content_name)
{
    if (!HasContentName(content_name)) {
        content_names_.push_back(content_name);
    }

}

bool ContentGroup::HasContentName(const std::string& content_name)
{
    for (auto name : content_names_) {
        if (content_name == name) {
            return true;
        }
    }
    return false;
}

VideoContentDescription::VideoContentDescription()
{

}

AudioContentDescription::AudioContentDescription()
{
}

} // namespace xrtc