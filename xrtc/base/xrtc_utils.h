#ifndef XRTCSDK_XRTC_BASE_XRTC_UTILS_H_
#define XRTCSDK_XRTC_BASE_XRTC_UTILS_H_

//通用函数集

#include <string>
#include <map>

namespace xrtc {

// xrtc://www.str2num.com/push?uid=xxx&streamName=xxx
bool ParseUrl(const std::string& url,
    std::string& protocol,
    std::string& host,
    std::string& action,
    std::map<std::string, std::string>& request_params);

} // namespace xrtc

#endif // XRTCSDK_XRTC_BASE_XRTC_UTILS_H_