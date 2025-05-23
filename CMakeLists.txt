cmake_minimum_required(VERSION 3.15)

set(app_name "ahrs_client")

# Set project name
project(${app_name})

# 设置所有可执行文件的运行时输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{FmDev}/bin)

#
include_directories($ENV{FmDev}/source)
include_directories($ENV{FmDev}/source/device)
include_directories($ENV{FmDev}/source/device/TDK40607P)
include_directories($ENV{FmDev}/libs/libgpiod/include)

#
# add_definitions(-DENABLE_LOGGING)
find_path(RAPIDCSV_INCLUDE_DIRS "rapidcsv.h")
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)


#
# 查找源文件
file(GLOB app_src
    $ENV{FmDev}/source/*.cpp
    $ENV{FmDev}/source/*.inl
    $ENV{FmDev}/source/*/*.cpp
    $ENV{FmDev}/source/*/*.inl
    $ENV{FmDev}/source/*/*.c
    $ENV{FmDev}/source/*/*/*.cpp
    $ENV{FmDev}/source/*/*/*.inl
    $ENV{FmDev}/source/*/*/*.c
    $ENV{FmDev}/source/*/*/*/*.cpp
    $ENV{FmDev}/source/*/*/*/*.inl
    $ENV{FmDev}/source/*/*/*/*.c
)

# 添加可执行文件
add_executable(${app_name}
    ${app_src}
)

# 链接库到可执行文件
target_link_libraries(${app_name} PUBLIC
    unofficial::concurrentqueue::concurrentqueue
    nlohmann_json::nlohmann_json
    #
    $ENV{FmDev}/libs/libgpiod/lib/libgpiod.a
)