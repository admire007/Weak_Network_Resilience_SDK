cmake_minimum_required(VERSION 3.8)

project(xrtc)

file(GLOB all_src
"./*.cpp"
"./base/*.cpp"
"./device/*.cpp"
"./media/base/*.cpp"
"./media/chain/*.cpp"
"./media/source/*.cpp"
"./media/sink/*.cpp"

)

#从根目录下开始查找文件
include_directories(
	${XRTC_DIR}
	${XRTC_THIRD_PARTY_DIR}/include
)

link_directories(
	${XRTC_THIRD_PARTY_DIR}/lib
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_definitions(-DXRTC_API_EXPORT
	-DWEBRTC_WIN
	-DNOMINMAX
	-DWIN32_LEAN_AND_MEAN
	-DCURL_STATICLIB
	-DICE_WIN
	)
endif()

add_library(xrtc SHARED ${all_src} "xrtc.cpp" "xrtc.h" "device/cam_impl.cpp" "device/cam_impl.h" "device/xrtc_render.h"   "media/base/media_chain.cpp" "media/base/media_chain.h" "media/chain/xrtc_preview.cpp" "media/chain/xrtc_preview.h" "media/source/xrtc_video_source.cpp" "media/source/xrtc_video_source.h" "media/sink/d3d9_render_sink.cpp" "media/sink/d3d9_render_sink.h" "media/base/base_pin.h" "media/base/in_pin.cpp" "media/base/in_pin.h" "media/base/out_pin.cpp" "media/base/out_pin.h" "base/xrtc_json.cpp" "base/xrtc_json.h")
target_link_libraries(xrtc
    absl_bad_optional_access
    absl_throw_delegate
    absl_strings
    absl_bad_variant_access
	winmm
	ws2_32
	Strmiids
	jpeg-static
	yuv
	jsoncpp_static
	d3d9
	libx264
	wldap32
	Crypt32
	iphlpapi
	libssl
	libcrypto
	libcurl
	libice
libwebrtc
)
