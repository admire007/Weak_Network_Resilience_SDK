#include "xrtc/xrtc.h"

#include <rtc_base/logging.h>
#include <rtc_base/task_utils/to_queued_task.h>
#include "xrtc/base/xrtc_global.h"


namespace xrtc {
	void xrtc::XRTCEngine::Init()
	{
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
	
}