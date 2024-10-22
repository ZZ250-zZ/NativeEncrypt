# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
# 指定构建该项目所需的CMake最低版本
cmake_minimum_required (VERSION 3.8)

project ("CMakeEncrypt")

# 根据平台设置 PLATFORM_INCLUDE_DIR 变量，然后使用 include_directories 命令添加头文件
if(WIN32)
    set(PLATFORM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/win32")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/linux")

	# 设置静态链接选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ libstdc")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
include_directories(${PLATFORM_INCLUDE_DIR})

# 导入公共 头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# 设置 vcpkg 路径
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# 查找 libcurl
find_package(CURL CONFIG REQUIRED)

# 定义动态库，NativeEncrypt 是 dll 文件名
add_library(NativeEncrypt SHARED src/CMakeEncrypt.cpp)

set(CURL_DIR "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/share/curl")
set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows")
# 链接 libcurl
target_link_libraries(NativeEncrypt PRIVATE CURL::libcurl)
