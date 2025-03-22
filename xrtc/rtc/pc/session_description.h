#ifndef XRTCSDK_XRTC_RTC_PC_SESSION_DESCRIPTION_H_
#define XRTCSDK_XRTC_RTC_PC_SESSION_DESCRIPTION_H_

#include <string>
#include <vector>
#include <memory>

#include <api/media_types.h>
#include <ice/candidate.h>
#include <ice/ice_credentials.h>


namespace xrtc {

enum class SdpType {
    kOffer,
    kAnswer,
};

class ContentGroup {
public:
    ContentGroup(const std::string& semantics) : semantics_(semantics) {}
    ~ContentGroup() {}

    std::string semantics() const { return semantics_; }
    const std::vector<std::string>& content_names() const { return content_names_; }
    void AddContentName(const std::string& content_name);
    bool HasContentName(const std::string& content_name);

private:
    std::string semantics_;
    std::vector<std::string> content_names_;
};

class TransportDescription {
public:
    std::string mid;
    std::string ice_ufrag;
    std::string ice_pwd;
};

//定义m行，结构信息保存到此
class MediaContentDescription {
public:
    virtual ~MediaContentDescription() {}
    virtual webrtc::MediaType type() = 0;
    virtual std::string mid() = 0;

    const std::vector<ice::Candidate>& candidates() {
        return candidates_;
    }

    void AddCandidate(const ice::Candidate& c) {
        candidates_.push_back(c);
    }

protected:
    std::vector<ice::Candidate> candidates_;

};

class AudioContentDescription : public MediaContentDescription {
public:
    AudioContentDescription();

    webrtc::MediaType type() override { return webrtc::MediaType::AUDIO; }
    std::string mid() override { return "audio"; }
};

class VideoContentDescription : public MediaContentDescription {
public:
    VideoContentDescription();

    webrtc::MediaType type() override { return webrtc::MediaType::VIDEO; }
    std::string mid() override { return "video"; }
};

class SessionDescription {
public:
    SessionDescription(SdpType type);
    ~SessionDescription();
    
    void AddContent(std::shared_ptr<MediaContentDescription> content) {
        contents_.push_back(content);
    }

    void AddGroup(const ContentGroup& group) {
        content_groups_.push_back(group);
    }

    std::shared_ptr<TransportDescription> GetTransportInfo(const std::string transport_name);
    void AddTransportInfo(std::shared_ptr<TransportDescription> td);
    void AddTransportInfo(const std::string& mid, const ice::IceParameters& ice_param);

private:
    SdpType sdp_type_;
    std::vector < std::shared_ptr<MediaContentDescription>> contents_;
    std::vector<ContentGroup> content_groups_;
    std::vector<std::shared_ptr<TransportDescription>> transport_info_;
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_PC_SESSION_DESCRIPTION_H_