﻿#include "xrtc/rtc/pc/peer_connection.h"

#include <vector>

#include <rtc_base/logging.h>
#include <rtc_base/string_encode.h>
#include <rtc_base/helpers.h>
#include <ice/candidate.h>

#include "xrtc/base/xrtc_global.h"

namespace xrtc {

PeerConnection::PeerConnection() :
    transport_controller_(std::make_unique<TransportController>())
  
{
}

PeerConnection::~PeerConnection() {
  
}

//a=attr_name:attr_value
static std::string GetAttribute(const std::string& line) {
    std::vector<std::string> fields;
    size_t size = rtc::tokenize(line,':',&fields);
    if (size != 2) {
        RTC_LOG(LS_WARNING) << "get attribute failed: " << line;
        return "";

    }
    return fields[1];
}

static bool ParseCandidates(MediaContentDescription* media_content,
    const std::string& line)
{
    if (line.find("a=candidate:") == std::string::npos) {
        return true;
    }

    std::string attr_value = GetAttribute(line);
    if (attr_value.empty()) {
        return false;
    }

    std::vector<std::string> fields;
    size_t size = rtc::tokenize(attr_value, ' ', &fields);
    if (size < 8) {
        return false;
    }

    ice::Candidate c;
    c.foundation = fields[0];
    c.component = std::atoi(fields[1].c_str());
    c.protocol = fields[2];
    c.priority = std::atoi(fields[3].c_str());
    c.port = std::atoi(fields[5].c_str());
    c.address = rtc::SocketAddress(fields[4], c.port);
    c.type = fields[7];

    media_content->AddCandidate(c);
    return true;
}

static bool ParseTransportInfo(TransportDescription* td,
    const std::string& line)
{
    if (line.find("a=ice-ufrag") != std::string::npos) {
        td->ice_ufrag = GetAttribute(line);
        if (td->ice_ufrag.empty()) {
            return false;
        }
    }
    else if (line.find("a=ice-pwd") != std::string::npos) {
        td->ice_pwd = GetAttribute(line);
        if (td->ice_pwd.empty()) {
            return false;
        }
    }

    return true;
}

int PeerConnection::SetRemoteSDP(const std::string& sdp) {
    std::vector<std::string> fields;
    // SDP用\n, \r\n来换行的
    rtc::tokenize(sdp, '\n', &fields);
    if (fields.size() <= 0) {
        RTC_LOG(LS_WARNING) << "invalid sdp: " << sdp;
        return -1;
    }

    // 判断是否是\r\n换行
    bool is_rn = false;
    if (sdp.find("\r\n") != std::string::npos) {
        is_rn = true;
    }

    remote_desc_ = std::make_unique<SessionDescription>(SdpType::kOffer);

    std::string mid;
    //保存M行的信息
    auto audio_content = std::make_shared<AudioContentDescription>();
    auto video_content = std::make_shared<VideoContentDescription>();
    //保存a=ice-ufrag  ice-pwd信息
    auto audio_td = std::make_shared<TransportDescription>();
    auto video_td = std::make_shared<TransportDescription>();

    for (auto field : fields) {
        // 如果以\r\n换行，去掉尾部的\r
        if (is_rn) {
            field = field.substr(0, field.length() - 1);
        }


        if (field.find("a=group:BUNDLE") != std::string::npos) {
            std::vector<std::string> items;
            /*例如，如果field是"a=group:BUNDLE audio video"，那么items将包含{"a=group:BUNDLE", "audio", "video"}。*/
            rtc::tokenize(field, ' ', &items);//使用rtc::tokenize函数将field字符串按照空格分割成多个子字符串，并存储到items向量中。
            if (items.size() > 1) {
                /*创建一个ContentGroup对象offer_bundle，并将其类型设置为"BUNDLE"。
ContentGroup类通常用于表示SDP中的组信息，比如BUNDLE组。*/
                ContentGroup offer_bundle("BUNDLE");
                for (size_t i = 1; i < items.size(); ++i) {
                    //遍历分割后的items向量，从索引1开始（跳过第一个元素"a=group:BUNDLE"）。
                    offer_bundle.AddContentName(items[i]);
                }
                //将BUNDLE组添加到远程描述
                remote_desc_->AddGroup(offer_bundle);
            }
        }
            else if (field.find_first_of("m=") == 0) {
                std::vector<std::string> items;
                rtc::tokenize(field, ' ', &items);
                if (items.size() <= 2) {
                    RTC_LOG(LS_WARNING) << "parse m= failed: " << field;
                    return -1;
                }

                 //m=audio/video
                mid = items[0].substr(2);
                if (mid == "audio") {
                    remote_desc_->AddContent(audio_content);
                    audio_td->mid = mid;
                }
                else if (mid == "video") {
                    remote_desc_->AddContent(video_content);
                    video_td->mid = mid;
                }
            }
            
            if ("audio" == mid) {
                if (!ParseCandidates(audio_content.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse candidate failed: " << field;
                    return -1;
                }

                if (!ParseTransportInfo(audio_td.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse transport info failed: " << field;
                    return -1;
                }
            }
            else if ("video" == mid) {
                if (!ParseCandidates(video_content.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse candidate failed: " << field;
                    return -1;
                }

                if (!ParseTransportInfo(video_td.get(), field)) {
                    RTC_LOG(LS_WARNING) << "parse transport info failed: " << field;
                    return -1;
                }
            }
        }

        remote_desc_->AddTransportInfo(audio_td);
        remote_desc_->AddTransportInfo(video_td);

        //if (video_content) {
        //    auto video_codecs = video_content->codecs();
        //    if (!video_codecs.empty()) {
        //        video_pt_ = video_codecs[0]->id;
        //    }

        //    if (video_codecs.size() > 1) {
        //        video_rtx_pt_ = video_codecs[1]->id;
        //    }
        //}

        transport_controller_->SetRemoteSDP(remote_desc_.get());//将解析的sdp设置到transport_controller

        return 0;
    }

    static RtpDirection GetDirection(bool send, bool recv) {
        if (send && recv) {
            return RtpDirection::kSendRecv;
        }
        else if (send && !recv) {
            return RtpDirection::kSendOnly;
        }
        else if (!send && recv) {
            return RtpDirection::kRecvOnly;
        }
        else {
            return RtpDirection::kInactive;
        }
    }

    std::string PeerConnection::CreateAnswer(const RTCOfferAnswerOptions& options, const std::string& stream_id)
    {
        local_desc_ = std::make_unique<SessionDescription>(SdpType::kAnswer);

        ice::IceParameters ice_param = ice::IceCredentials::CreateRandomIceCredentials();
        std::string cname = rtc::CreateRandomString(16);

        if (options.send_audio || options.recv_audio) {
            auto audio_content = std::make_shared<AudioContentDescription>();
            audio_content->set_direction(GetDirection(options.send_audio, options.recv_audio));
            audio_content->set_rtcp_mux(options.use_rtcp_mux);
            local_desc_->AddContent(audio_content);
            local_desc_->AddTransportInfo(audio_content->mid(),ice_param);
            
            // 如果发送音频，需要创建stream
              if (options.send_audio) {
                  StreamParams audio_stream;
                  audio_stream.id = rtc::CreateRandomString(16);
                  audio_stream.stream_id = stream_id;
                  audio_stream.cname = cname;
                  local_audio_ssrc_ = rtc::CreateRandomId();
                  audio_stream.ssrcs.push_back(local_audio_ssrc_);
                  audio_content->AddStream(audio_stream);

        
              }
        }

        if (options.send_video || options.recv_video) {
            auto video_content = std::make_shared<VideoContentDescription>();
            video_content->set_direction(GetDirection(options.send_video, options.recv_video));
            video_content->set_rtcp_mux(options.use_rtcp_mux);
            local_desc_->AddContent(video_content);
            local_desc_->AddTransportInfo(video_content->mid(), ice_param);
         
            // 如果发送视频，需要创建stream
            if (options.send_video) {
                std::string id = rtc::CreateRandomString(16);
                StreamParams video_stream;
                video_stream.id = id;
                video_stream.stream_id = stream_id;
                video_stream.cname = cname;
                local_video_ssrc_ = rtc::CreateRandomId();
                local_video_rtx_ssrc_ = rtc::CreateRandomId();
                video_stream.ssrcs.push_back(local_video_ssrc_);
                video_stream.ssrcs.push_back(local_video_rtx_ssrc_);
                

                // 分组
                SsrcGroup sg;
                sg.semantics = "FID";
                sg.ssrcs.push_back(local_video_ssrc_);
                sg.ssrcs.push_back(local_video_rtx_ssrc_);
                video_stream.ssrc_groups.push_back(sg);

                video_content->AddStream(video_stream);

                // 创建rtx stream
                StreamParams video_rtx_stream;
                video_rtx_stream.id = id;
                video_rtx_stream.stream_id = stream_id;
                video_rtx_stream.cname = cname;
                video_rtx_stream.ssrcs.push_back(local_video_rtx_ssrc_);
                video_content->AddStream(video_rtx_stream);
            }
        
        }


        // 创建BUNDLE
        if (options.use_rtp_mux) {
            ContentGroup answer_bundle("BUNDLE");
            for (auto content : local_desc_->contents()) {
                answer_bundle.AddContentName(content->mid());
            }

            if (!answer_bundle.content_names().empty()) {
                local_desc_->AddGroup(answer_bundle);
            }
        }


        return local_desc_->ToString();//文本转换为SDP格式
    }

} // namespace xrtc