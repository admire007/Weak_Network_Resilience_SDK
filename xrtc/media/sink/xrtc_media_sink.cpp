#include "xrtc/media/sink/xrtc_media_sink.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>

#include "xrtc/base/xrtc_global.h"
#include "xrtc/base/xrtc_json.h"
#include "xrtc/media/base/in_pin.h"
#include "xrtc/base/xrtc_utils.h"
#include "xrtc/base/xrtc_http.h"


namespace xrtc {

XRTCMediaSink::XRTCMediaSink(MediaChain* media_chain) :
    media_chain_(media_chain),
    video_in_pin_(std::make_unique<InPin>(this)),
    pc_(std::make_unique<PeerConnection>())
{
    MediaFormat video_fmt;
    video_fmt.media_type = MainMediaType::kMainTypeVideo;
    video_fmt.sub_fmt.video_fmt.type = SubMediaType::kSubTypeH264;
    video_in_pin_->set_format(video_fmt);

    XRTCGlobal::Instance()->http_manager()->AddObject(this);//将当前对象加入http_manager进行管理

}

XRTCMediaSink::~XRTCMediaSink() {
    XRTCGlobal::Instance()->http_manager()->RemoveObject(this);
}

bool XRTCMediaSink::Start() {
    RTC_LOG(LS_INFO) << "XRTCMediaSink Start";
    // 解析Url
    if (!ParseUrl(url_, protocol_, host_, action_, request_params_)) {
        return false;
    }

    if (action_ != "push" || request_params_["uid"].empty() || request_params_["streamName"].empty()) {
        RTC_LOG(LS_WARNING) << "invalid url: " << url_;
        return false;
    }

    // 发送信令请求
    // https://www.str2num.com/signaling/push?uid=xxx&streamName=xxx&audio=1&video=1&isDtls=0
    // 构造body
    std::stringstream body;
    body << "uid=" << request_params_["uid"]
        << "&streamName=" << request_params_["streamName"]
        << "&audio=1&video=1&isDtls=0";
    std::string url = "https://" + host_ + "/signaling/push";
    HttpRequest request(url, body.str());//使用构造好的URL和请求体字符串，创建一个HttpRequest对象。

    // 发送请求
    XRTCGlobal::Instance()->http_manager()->Post(request, [=](HttpReply reply) {
        RTC_LOG(LS_INFO) << "signaling push response, url: " << reply.get_url()
            << ", body: " << reply.get_body()
            << ", status: " << reply.get_status_code()
            << ", err_no: " << reply.get_errno()
            << ", err_msg: " << reply.get_err_msg()
            << ", response: " << reply.get_resp();

        //用于存储从服务器响应中解析出的类型和SDP数据
        std::string type;
        std::string sdp;

        if (!ParseReply(reply, type, sdp)) {
            if (media_chain_) {
                media_chain_->OnChainFailed(this, XRTCError::kPushRequestOfferErr);
            }
            return;
        }

        if (pc_->SetRemoteSDP(sdp) != 0) {
            return;
        }

        //因为不需要拉流，所以禁用
        RTCOfferAnswerOptions options;
        options.recv_audio = false;
        options.recv_video = false;
        std::string answer = pc_->CreateAnswer(options,request_params_["uid_"]);
        SendAnswer(answer);

    }, this);

    return true;
}

void XRTCMediaSink::Setup(const std::string& json_config) {
    JsonValue value;
    value.FromJson(json_config);
    JsonObject jobj = value.ToObject();
    JsonObject jxrtc_media_sink = jobj["xrtc_media_sink"].ToObject();
    url_ = jxrtc_media_sink["url"].ToString();
}

void XRTCMediaSink::Stop() {
    RTC_LOG(LS_INFO) << "XRTCMediaSink Stop";
    // 向后台服务发送停止推流请求
    SendStop();
}

void XRTCMediaSink::OnNewMediaFrame(std::shared_ptr<MediaFrame> frame) {
    // 通过网络线程，将x264压缩后的数据发送到服务器
    XRTCGlobal::Instance()->network_thread()->PostTask(webrtc::ToQueuedTask([=]() {
        if (MainMediaType::kMainTypeVideo == frame->fmt.media_type) {
            PacketAndSendVideo(frame);
        }
    }));
}


bool XRTCMediaSink::ParseReply(const HttpReply& reply, std::string& type,
    std::string& sdp) 
{
    if (reply.get_status_code() != 200 || reply.get_errno() != 0) {
        RTC_LOG(LS_WARNING) << "signaling response error";
        return false;
    }

    JsonValue value;
    if (!value.FromJson(reply.get_resp())) {
        RTC_LOG(LS_WARNING) << "invalid json response";
        return false;
    }

    JsonObject jobj = value.ToObject();
    int err_no = jobj["errNo"].ToInt();
    if (err_no != 0) {
        RTC_LOG(LS_WARNING) << "response errNo is not 0, err_no: " << err_no;
        return false;
    }

    JsonObject data = jobj["data"].ToObject();
    type = data["type"].ToString();
    sdp = data["sdp"].ToString();

    if (sdp.empty()) {
        RTC_LOG(LS_WARNING) << "sdp is empty";
        return false;
    }

    return true;
}

void XRTCMediaSink::SendAnswer(const std::string& answer) {
    if (request_params_["uid"].empty() || request_params_["streamName"].empty()) {
        RTC_LOG(LS_WARNING) << "send answer failed, invalid url: " << url_;
        return;
    }

    // 构造body
    std::stringstream body;
    body << "uid=" << request_params_["uid"]
        << "&streamName=" << request_params_["streamName"]
        << "&type=push"
        << "&answer=" << HttpManager::UrlEncode(answer);

    std::string url = "https://" + host_ + "/signaling/sendanswer";
    HttpRequest request(url, body.str());
    // 发送请求
    XRTCGlobal::Instance()->http_manager()->Post(request, [=](HttpReply reply) {
        RTC_LOG(LS_INFO) << "signaling sendanswer response, url: " << reply.get_url()
            << ", body: " << reply.get_body()
            << ", status: " << reply.get_status_code()
            << ", err_no: " << reply.get_errno()
            << ", err_msg: " << reply.get_err_msg()
            << ", response: " << reply.get_resp();

        if (reply.get_status_code() != 200 || reply.get_errno() != 0) {
            RTC_LOG(LS_WARNING) << "signaling sendanswer response error";
            return;
        }

        JsonValue value;
        if (!value.FromJson(reply.get_resp())) {
            RTC_LOG(LS_WARNING) << "invalid json response";
            return;
        }

        JsonObject jobj = value.ToObject();
        int err_no = jobj["errNo"].ToInt();
        if (err_no != 0) {
            RTC_LOG(LS_WARNING) << "response errNo is not 0, err_no: " << err_no;
            return;
        }

     }, this);
}

void XRTCMediaSink::SendStop() {
    // 发送停止推流的信令请求
    // https://www.str2num.com/signaling/stoppush?uid=xxx&streamName=xxx
    // 构造body
    std::stringstream body;
    body << "uid=" << request_params_["uid"]
        << "&streamName=" << request_params_["streamName"];
    std::string url = "https://" + host_ + "/signaling/stoppush";
    HttpRequest request(url, body.str());

    // 发送请求
    XRTCGlobal::Instance()->http_manager()->Post(request, [=](HttpReply reply) {
        RTC_LOG(LS_INFO) << "signaling stoppush response, url: " << reply.get_url()
            << ", body: " << reply.get_body()
            << ", status: " << reply.get_status_code()
            << ", err_no: " << reply.get_errno()
            << ", err_msg: " << reply.get_err_msg()
            << ", response: " << reply.get_resp();

        if (reply.get_status_code() != 200 || reply.get_errno() != 0) {
            RTC_LOG(LS_WARNING) << "signaling stoppush response error";
            return;
        }

        JsonValue value;
        if (!value.FromJson(reply.get_resp())) {
            RTC_LOG(LS_WARNING) << "invalid json response";
            return;
        }

        JsonObject jobj = value.ToObject();
        int err_no = jobj["errNo"].ToInt();
        if (err_no != 0) {
            RTC_LOG(LS_WARNING) << "response errNo is not 0, err_no: " << err_no;
            return;
        }

    }, this);
}

void XRTCMediaSink::PacketAndSendVideo(std::shared_ptr<MediaFrame> frame)
{

}



} // namespace xrtc