cmake_minimum_required(VERSION 3.16)
project (RPCameraInterface VERSION 1.2)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_DEBUG_POSTFIX "d")

option(USE_DEPTHAI "Use DepthAI" OFF)

find_package(BufferedSocket REQUIRED)
message("BufferedSocket dir: ${BufferedSocket_DIR}")
set(BufferedSocket_INCLUDE_DIR ${BufferedSocket_DIR}/../../../include)
include_directories(${BufferedSocket_INCLUDE_DIR})

find_package(RPCameraInterface REQUIRED)
message("RPCameraInterface include dir: ${RPCameraInterface_DIR}")
set(RPCameraInterface_INCLUDE_DIR ${RPCameraInterface_DIR}/../../../include)
include_directories(${RPCameraInterface_INCLUDE_DIR})

if(USE_DEPTHAI)
	find_package(depthai CONFIG REQUIRED)
endif()

find_package(OpenCV REQUIRED videoio highgui)

add_executable(RPCameraInterfaceDemo main.cpp)

target_link_libraries(RPCameraInterfaceDemo PRIVATE ${OpenCV_LIBS})
target_link_libraries(RPCameraInterfaceDemo LINK_PUBLIC RPCameraInterface)

install(TARGETS RPCameraInterfaceDemo RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
