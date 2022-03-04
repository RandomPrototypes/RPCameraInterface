cmake_minimum_required(VERSION 3.16)
project (RPCameraInterface VERSION 0.1)
set(CMAKE_CXX_STANDARD 11)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(FetchContent)
include(GNUInstallDirs)

option(USE_QT "Use QT" OFF)
option(USE_OPENCV "Use OpenCV" OFF)
option(BUILD_DEMO "Build DEMO" OFF)

set(EXTERNAL_INSTALL_LOCATION deps)

FetchContent_Populate(
    BufferedSocket
    GIT_REPOSITORY https://github.com/RandomPrototypes/BufferedSocket.git
    GIT_TAG 45c3d9e33f20aafa19ec4f03d39dcbc9c529726a
)
SET(BufferedSocket ${bufferedsocket_SOURCE_DIR})

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
   include_directories(${FFMPEG_INCLUDE_DIR})
   link_directories(${FFMPEG_LIB_DIR})
else()
   find_package(PkgConfig REQUIRED)
   pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
   )
endif()

set(LIB_SRC
	src/CameraInterface.cpp
	src/ImageData.cpp
	src/ImageFormat.cpp
	src/ImageFormatConverter.cpp
	src/CameraInterfaceAndroid.cpp
	${BufferedSocket}/BufferedSocket.h
        ${BufferedSocket}/BufferedSocket.cpp
)

set(LIB_INCLUDE
	include/RPCameraInterface/CameraInterface.h
	include/RPCameraInterface/ImageData.h
	include/RPCameraInterface/ImageFormat.h
	include/RPCameraInterface/ImageFormatConverter.h
	include/RPCameraInterface/CameraInterfaceAndroid.h
)

if(${USE_QT})
    set(LIB_SRC
	${LIB_SRC}
	src/CameraEnumeratorQt.cpp
    )
    set(LIB_INCLUDE
	${LIB_INCLUDE}
	include/RPCameraInterface/CameraEnumeratorQt.h
    )
endif()

if(${USE_OPENCV})
    find_package(OpenCV REQUIRED)
    set(LIB_SRC
	${LIB_SRC}
	src/CameraInterfaceOpenCV.cpp
    )
    set(LIB_INCLUDE
	${LIB_INCLUDE}
	include/RPCameraInterface/CameraInterfaceOpenCV.h
    )
endif()

check_include_file(dshow.h HAVE_DSHOW)

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${BufferedSocket})

add_library(RPCameraInterface SHARED ${LIB_SRC} ${LIB_INCLUDE})
set_target_properties(RPCameraInterface PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(RPCameraInterface PROPERTIES PUBLIC_HEADER ${LIB_INCLUDE})

if(BUILD_DEMO)
   add_executable(RPCameraInterfaceDemo ${LIB_INCLUDE} demo/main.cpp)
endif()

if(HAVE_DSHOW)
   target_compile_definitions(RPCameraInterface PUBLIC "HAVE_DSHOW")
endif()

target_link_libraries(RPCameraInterface PRIVATE 
	libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil)
if(WIN32)
   target_link_libraries(RPCameraInterface PRIVATE ws2_32)
endif()
if(${USE_OPENCV})
   target_link_libraries(RPCameraInterface PRIVATE ${OpenCV_LIBS})
endif()

if(BUILD_DEMO)
   target_link_libraries(RPCameraInterfaceDemo PRIVATE ${OpenCV_LIBS})
   target_link_libraries(RPCameraInterfaceDemo LINK_PUBLIC RPCameraInterface)
endif()

install(TARGETS RPCameraInterface  EXPORT RPCameraInterface-config
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
#install(TARGETS RPCameraInterface DESTINATION lib/ EXPORT RPCameraInterface-config)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION . PATTERN "*.h")	
install(EXPORT RPCameraInterface-config DESTINATION lib/cmake/RPCameraInterface)
export(TARGETS RPCameraInterface FILE RPCameraInterface-config.cmake)
