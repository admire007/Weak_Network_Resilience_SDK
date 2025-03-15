cmake_minimum_required(VERSION 3.8)

project(windemo)

include_directories(
${XRTC_DIR}
${XRTC_THIRD_PARTY_DIR}/include
${XRTC_DIR}/../XRTC_third_part/NIM_Duilib_Framework
)

#指定链接库路径
link_directories(
${XRTC_THIRD_PARTY_DIR}
${XRTC_THIRD_PARTY_DIR}/lib
${XRTC_DIR}/../XRTC_third_part/out/build/x64-Debug/Window/lib

)

file(GLOB all_src
./src/*.cpp
)
# 定义一个名为 windemo 的可执行文件目标。
#使用变量 ${all_src} 中定义的源文件列表来编译生成该可执行文件。
add_executable(windemo ${all_src})

#链接库
target_link_libraries(windemo
base
duilib 
Imm32
comctl32
xrtc
)

#设置为windows程序而非控制台程序
set_target_properties(windemo PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /SAFESEH:NO")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})