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





	class XRTC_API XRTCEngine {
	public:
		static void Init();
		// 视频设备
		static uint32_t GetGameraCount();
		static int32_t GetCameraInfo(int index, std::string& device_name,
			std::string& device_id);
		/*static IVideoSource* CreateCamSource(const std::string& cam_id);*/
		
		// 音频设备
		//static int16_t GetMicCount();
		//static int32_t GetMicInfo(int index, std::string& mic_name, std::string& mic_guid);
		//static IAudioSource* CreateMicSource(const std::string& mic_id);
		//static XRTCRender* CreateRender(void* canvas);
		//static XRTCPreview* CreatePreview(IVideoSource* video_source, XRTCRender* render);
		//static XRTCPusher* CreatePusher(IAudioSource* audio_source, IVideoSource* video_source);

	};
}

#endif // ! XRTCSDK_XRTC_XRTC_H
