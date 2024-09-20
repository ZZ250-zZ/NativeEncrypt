# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("CMakeEncrypt")

# 根据平台设置 PLATFORM_INCLUDE_DIR 变量，然后使用 include_directories 命令将其添加到编译器的包含路径中
if(WIN32)
    set(PLATFORM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/win32")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/linux")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
include_directories(${PLATFORM_INCLUDE_DIR})

# 导入公共 头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# 定义动态库，NativeEncrypt 是 dll 文件名
add_library(NativeEncrypt SHARED src/CMakeEncrypt.cpp)
