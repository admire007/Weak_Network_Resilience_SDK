#include "xrtc/xrtc.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include "xrtc/base/xrtc_global.h"
#include "xrtc/device/cam_impl.h"


namespace xrtc {
	void xrtc::XRTCEngine::Init()
		
	{
		rtc::LogMessage::LogTimestamps(true);//显示程序启动时间
		rtc::LogMessage::LogThreads(true);
		rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
		
		RTC_LOG(LS_INFO) << "XTRCSDK init";
	}
	uint32_t XRTCEngine::GetGameraCount()
	{
		return XRTCGlobal::Instance()->api_thread()->Invoke<uint32_t>(RTC_FROM_HERE, [=]() {
			return XRTCGlobal::Instance()->video_device_info()->NumberOfDevices();
			});

	}
	int32_t XRTCEngine::GetCameraInfo(int index, std::string& device_name, std::string& device_id)
	{
		return XRTCGlobal::Instance()->api_thread()->Invoke<int32_t>(RTC_FROM_HERE, [&]() {
			char name[128];
			char id[128];
			int32_t res = XRTCGlobal::Instance()->video_device_info()->GetDeviceName(index,
				name, sizeof(name), id, sizeof(id));
			device_name = name;
			device_id = id;
			return res;
			});
	}

	IVideoSource* XRTCEngine::CreateCamSource(const std::string& cam_id) {
		return XRTCGlobal::Instance()->api_thread()->Invoke<IVideoSource*>(RTC_FROM_HERE, [=]() {
			return new CamImpl(cam_id);
			});

	}
	
}