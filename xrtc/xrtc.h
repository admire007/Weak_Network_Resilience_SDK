#ifndef XRTCSDK_XRTC_XRTC_H
#define XRTCSDK_XRTC_XRTC_H

#ifdef XRTC_STATIC
#define XRTC_API
#else
#ifdef XRTC_API_EXPORT
#if defined(_MSC_VER)
#define XRTC_API __declspec(dllexport)
#else
#define XRTC_API
#endif
#else
#if defined(_MSC_VER)
#define XRTC_API __declspec(dllimport)
#else
#define XRTC_API
#endif
#endif
#endif
#include "xrtc/xrtc.h"

#include "xrtc/base/xrtc_global.h"
#include <string>
#include <memory>


namespace xrtc
{
	class MediaFrame;
	class XRTCRender;
	class XRTCPreview;
	class XRTCPusher;

	enum class XRTCError {
		kNoErr = 0,
		kVideoCreateCaptureErr = -1000,
		kVideoNoCapabilitiesErr,
		kVideoNoBestCapabilitiesErr,
		kVideoStartCaptureErr,
		kPreviewNoVideoSourceErr,
		kChainConnectErr,
		kChainStartErr,
		kPushNoVideoSourceErr,
		kPushInvalidUrlErr,
		kPushRequestOfferErr,
		kPushIceConnectionErr,
		kNoAudioDeviceErr,
		kAudioNotFoundErr,
		kAudioSetRecordingDeviceErr,
		kAudioInitRecordingErr,
		kAudioStartRecordingErr,
	};
	
	//消耗帧
	class IXRTCConsumer {
	public:
		virtual ~IXRTCConsumer() {}
		virtual void OnFrame(std::shared_ptr<MediaFrame> frame) = 0;
	};

	//视频源
	class IMediaSource {
	public:
		virtual ~IMediaSource() {}
		virtual void Start() = 0;
		virtual void Setup(const std::string& json_config) = 0;//设置摄像头采集参数
		virtual void Stop() = 0;
		virtual void Destroy() = 0;

		virtual void AddConsumer(IXRTCConsumer* consumer) = 0;
		virtual void RemoveConsumer(IXRTCConsumer* consumer) = 0;


	};

	class IAudioSource : public IMediaSource {

	};

	class IVideoSource : public IMediaSource {

	};

	class XRTC_API XRTCEngineObserver {
	public:
		virtual void OnVideoSourceSuccess(IVideoSource*) {}
		virtual void OnVideoSourceFailed(IVideoSource*,XRTCError) {}
		virtual void OnPreviewSuccess(XRTCPreview*) {}
		virtual void OnPreviewFailed(XRTCPreview*, XRTCError) {}
		virtual void OnPushSuccess(XRTCPusher*) {}
		virtual void OnPushFailed(XRTCPusher*, XRTCError) {}
	};



	class XRTC_API XRTCEngine {
	public:
		static void Init(XRTCEngineObserver* observer);
		// 视频设备
		static uint32_t GetGameraCount();
		static int32_t GetCameraInfo(int index, std::string& device_name,
			std::string& device_id);
		static IVideoSource* CreateCamSource(const std::string& cam_id);
		static XRTCRender* CreateRender(void* canvan);
		static XRTCPreview* CreatePreview(IVideoSource* video_source,XRTCRender* render);//为了实现渲染，后续实现d3d9获取句柄时创建XRTCRender* render
		static XRTCPusher* CreatePusher(IVideoSource* video_source);



		// 音频设备
		//static int16_t GetMicCount();
		//static int32_t GetMicInfo(int index, std::string& mic_name, std::string& mic_guid);
		//static IAudioSource* CreateMicSource(const std::string& mic_id);
		//static XRTCPusher* CreatePusher(IAudioSource* audio_source, IVideoSource* video_source);

	};
}

#endif // ! XRTCSDK_XRTC_XRTC_H
