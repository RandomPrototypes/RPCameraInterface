cmake_minimum_required(VERSION 3.16)
project (RPCameraInterface VERSION 1.2)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_DEBUG_POSTFIX "d")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(FetchContent)
include(GNUInstallDirs)

check_include_file(dshow.h HAVE_DSHOW)
check_include_file(linux/videodev2.h HAVE_V4L2)

option(USE_QT "Use QT" OFF)
option(USE_OPENCV "Use OpenCV" ON)
option(USE_GSTREAMER "Use GStreamer" ON)
option(USE_DEPTHAI "Use DepthAI" OFF)
option(BUILD_DEMO "Build DEMO" OFF)

set(EXTERNAL_INSTALL_LOCATION deps)

find_package(BufferedSocket REQUIRED)
message("BufferedSocket dir: ${BufferedSocket_DIR}")
set(BufferedSocket_INCLUDE_DIR ${BufferedSocket_DIR}/../../../include)
include_directories(${BufferedSocket_INCLUDE_DIR})

if(WIN32)
   SET(FFMPEG_DIR "" CACHE STRING "ffmpeg dir")
   SET(FFMPEG_INCLUDE_DIR "" CACHE STRING "ffmpeg include dir")
   SET(FFMPEG_LIB_DIR "" CACHE STRING "ffmpeg lib dir")
   if(FFMPEG_DIR AND (NOT FFMPEG_INCLUDE_DIR))
      set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include CACHE STRING "ffmpeg include dir" FORCE)
   endif()
   if(FFMPEG_DIR AND (NOT FFMPEG_LIB_DIR))
      set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib CACHE STRING "ffmpeg lib dir" FORCE)
   endif()
   string(REGEX REPLACE "\\\\" "/" FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR})
   if(EXISTS ${FFMPEG_INCLUDE_DIR}/libavdevice/avdevice.h)
      set(HAVE_FFMPEG 1)
   endif()
   message(STATUS "    FFMPEG: ${HAVE_FFMPEG}")
   message(STATUS "    FFMPEG_INCLUDE_DIR: ${FFMPEG_INCLUDE_DIR}")
   message(STATUS "    FFMPEG_LIB_DIR: ${FFMPEG_LIB_DIR}")
   include_directories(${FFMPEG_INCLUDE_DIR})
   link_directories(${FFMPEG_LIB_DIR})
else()
   find_package(PkgConfig REQUIRED)
   pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET libavdevice libavfilter libavformat libavcodec libswresample libswscale libavutil)
endif()

if(USE_DEPTHAI)
	find_package(depthai CONFIG REQUIRED)
	set(depthai_INCLUDE_DIR ${depthai_DIR}/../../../include)
	include_directories(${depthai_INCLUDE_DIR})
	include_directories(${depthai_INCLUDE_DIR}/depthai-shared/3rdparty)
	include_directories(${libnop_DIR}/../../../include)
endif()

set(LIB_SRC
	src/CameraInterface.cpp
	src/CameraInterfaceBase.cpp
	src/ImageData.cpp
	src/ImageFormat.cpp
	src/ImageFormatConverter.cpp
	src/VideoEncoder.cpp
	src/CameraInterfaceRPNetwork.cpp
	src/PortableTypes.cpp
)

set(LIB_INCLUDE
	include/RPCameraInterface/CameraInterface.h
	include/RPCameraInterface/CameraInterfaceBase.h
	include/RPCameraInterface/ImageData.h
	include/RPCameraInterface/ImageFormat.h
	include/RPCameraInterface/ImageFormatConverter.h
	include/RPCameraInterface/VideoEncoder.h
	include/RPCameraInterface/CameraInterfaceRPNetwork.h
	include/RPCameraInterface/PortableTypes.h
	include/RPCameraInterface/OpenCVConverter.h
)

if(${USE_QT})
    set(LIB_SRC ${LIB_SRC} src/CameraEnumeratorQt.cpp)
    set(LIB_INCLUDE ${LIB_INCLUDE} include/RPCameraInterface/CameraEnumeratorQt.h)
endif()

if(${USE_GSTREAMER})
    if(WIN32)
         find_path(GSTREAMER_ROOT_INCLUDE_DIR gstreamer-1.0/gst/gst.h)
         find_path(GSTREAMER_ROOT_LIB_DIR gstreamer-1.0.lib)
         include_directories(${GSTREAMER_ROOT_INCLUDE_DIR}/gstreamer-1.0
                             ${GSTREAMER_ROOT_INCLUDE_DIR}/glib-2.0
                             ${GSTREAMER_ROOT_LIB_DIR}/glib-2.0/include)
         link_directories(${GSTREAMER_ROOT_LIB_DIR})
    else()
       #pkg_search_module(GLIB REQUIRED glib-2.0) 
       #pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
       #pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
       #pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)
       find_package(GStreamer COMPONENTS base)
       find_package(GStreamerPluginsBase COMPONENTS app video pbutils)
       find_package(GLIB2)
       find_package(GObject)
       include_directories(${GSTREAMER_INCLUDE_DIR} ${GSTREAMER_VIDEO_INCLUDE_DIR} ${GLIB2_INCLUDE_DIR})
    endif()
    set(LIB_SRC ${LIB_SRC} src/CameraInterfaceGStreamer.cpp)
    set(LIB_INCLUDE ${LIB_INCLUDE} include/RPCameraInterface/CameraInterfaceGStreamer.h)
endif()

if(${USE_OPENCV} OR ${BUILD_DEMO})
    find_package(OpenCV REQUIRED videoio)
    set(OpenCV_LIBS_minimal ${OpenCV_LIBS})
	if(BUILD_DEMO)
		find_package(OpenCV REQUIRED highgui)
	endif()
endif()
if(${USE_OPENCV})
    set(LIB_SRC ${LIB_SRC} src/CameraInterfaceOpenCV.cpp)
    set(LIB_INCLUDE ${LIB_INCLUDE} include/RPCameraInterface/CameraInterfaceOpenCV.h)
endif()

if(HAVE_DSHOW)
	message(STATUS "    DirectShow: YES")
	set(LIB_SRC
		${LIB_SRC}
		src/CameraInterfaceDShow.cpp
		src/cap_dshow/cap_dshow.cpp
    )
    set(LIB_INCLUDE
		${LIB_INCLUDE}
		include/RPCameraInterface/CameraInterfaceDShow.h
		src/cap_dshow/cap_dshow.hpp
    )
else()
    message(STATUS "    DirectShow: NO")
endif()

if(HAVE_V4L2)
	message(STATUS "    V4L2      : YES")
	set(LIB_SRC ${LIB_SRC} src/CameraInterfaceV4L2.cpp)
   set(LIB_INCLUDE ${LIB_INCLUDE} include/RPCameraInterface/CameraInterfaceV4L2.h)
else()
	message(STATUS "    V4L2      : NO")
endif()

if(USE_DEPTHAI)
	message(STATUS "    DepthAI   : YES")
	set(LIB_SRC ${LIB_SRC} src/CameraInterfaceDepthAI.cpp)
   set(LIB_INCLUDE ${LIB_INCLUDE} include/RPCameraInterface/CameraInterfaceDepthAI.h)
else()
	message(STATUS "    DepthAI   : NO")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(RPCameraInterface SHARED ${LIB_SRC} ${LIB_INCLUDE})
target_compile_definitions(RPCameraInterface PRIVATE "RPCAMERAINTERFACE_EXPORTS")

set_target_properties(RPCameraInterface PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(RPCameraInterface PROPERTIES PUBLIC_HEADER ${LIB_INCLUDE})

target_link_libraries(RPCameraInterface PRIVATE BufferedSocket)

if(BUILD_DEMO)
   add_executable(RPCameraInterfaceDemo ${LIB_INCLUDE} demo/main.cpp)
endif()

if(HAVE_DSHOW)
   target_compile_definitions(RPCameraInterface PUBLIC "HAVE_DSHOW")
endif()
if(HAVE_V4L2)
	target_compile_definitions(RPCameraInterface PUBLIC "HAVE_V4L2")
endif()
if(USE_DEPTHAI)
	target_compile_definitions(RPCameraInterface PUBLIC "USE_DEPTHAI")
endif()

if(USE_GSTREAMER)
	target_compile_definitions(RPCameraInterface PUBLIC "USE_GSTREAMER")
endif()

if(WIN32)
   if(MSVC)
      target_link_libraries(RPCameraInterface PRIVATE avdevice avfilter avformat avcodec swresample swscale avutil)
   else()
      target_link_libraries(RPCameraInterface PRIVATE libavdevice libavfilter libavformat libavcodec libswresample libswscale libavutil)
   endif()
   target_link_libraries(RPCameraInterface PRIVATE ws2_32)
else()
   target_link_libraries(RPCameraInterface PRIVATE PkgConfig::LIBAV)
endif()

if(${USE_GSTREAMER})
    if(WIN32)
       target_link_libraries(RPCameraInterface PUBLIC gstreamer-1.0 gstapp-1.0 gstvideo-1.0 gstpbutils-1.0 glib-2.0 gobject-2.0)
    else()
       target_link_libraries(RPCameraInterface PRIVATE ${GST_APP_LIBRARIES} ${GST_VIDEO_LIBRARIES} ${GSTREAMER_BASE_LIBRARY} ${GSTREAMER_LIBRARY} ${GSTREAMER_APP_LIBRARY} ${GSTREAMER_VIDEO_LIBRARY} ${GSTREAMER_PBUTILS_LIBRARY})
       target_link_libraries(RPCameraInterface PRIVATE ${GLIB2_LIBRARIES} ${GOBJECT_LIBRARIES})
    endif()
endif()

if(${USE_OPENCV})
   target_link_libraries(RPCameraInterface PRIVATE ${OpenCV_LIBS_minimal})
endif()

if(USE_DEPTHAI)
	target_link_libraries(RPCameraInterface PUBLIC depthai::opencv)
endif()

if(BUILD_DEMO)
   target_link_libraries(RPCameraInterfaceDemo PRIVATE ${OpenCV_LIBS})
   target_link_libraries(RPCameraInterfaceDemo LINK_PUBLIC RPCameraInterface)
endif()

install(TARGETS RPCameraInterface  EXPORT rpcamerainterface-config
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
#install(TARGETS RPCameraInterface DESTINATION lib/ EXPORT rpcamerainterface-config)

if(BUILD_DEMO)
   install(TARGETS RPCameraInterfaceDemo RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()


install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION . PATTERN "*.h")	
install(EXPORT rpcamerainterface-config DESTINATION lib/cmake/RPCameraInterface)
export(TARGETS RPCameraInterface FILE rpcamerainterface-config.cmake)
