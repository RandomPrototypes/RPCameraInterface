cmake_minimum_required(VERSION 2.8.12)
project (RPCameraInterface)
set(CMAKE_CXX_STANDARD 11)

include(CheckIncludeFile)
include(CheckIncludeFiles)

option(USE_QT "Use QT" OFF)
option(USE_OPENCV "Use OpenCV" OFF)

SET(FFMPEG_DIR C:/ffmpeg5.0 CACHE STRING "ffmpeg dir")
include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)

set(LIB_SRC
	src/CameraInterface.cpp
	src/ImageData.cpp
	src/ImageFormat.cpp
	src/ImageFormatConverter.cpp
)

set(LIB_INCLUDE
	include/CameraInterface.h
	include/ImageData.h
	include/ImageFormat.h
	include/ImageFormatConverter.h
)

if(${USE_QT})
    set(LIB_SRC
	${LIB_SRC}
	src/CameraEnumeratorQt.cpp
    )
    set(LIB_INCLUDE
	${LIB_INCLUDE}
	include/CameraEnumeratorQt.h
    )
endif()

if(${USE_OPENCV})
    find_package(OpenCV REQUIRED HINTS C:/opencv-4.5.5/release/install/x64/mingw/lib)
    set(LIB_SRC
	${LIB_SRC}
	src/CameraInterfaceOpenCV.cpp
    )
    set(LIB_INCLUDE
	${LIB_INCLUDE}
	include/CameraInterfaceOpenCV.h
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
	include/CameraInterfaceDShow.h
	src/cap_dshow/cap_dshow.hpp
    )
else()
    message(STATUS "    DirectShow: NO")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(RPCameraInterface STATIC ${LIB_SRC} ${LIB_INCLUDE})
add_executable(RPCameraInterfaceDemo ${LIB_INCLUDE} demo/main.cpp)

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
if(${USE_OPENCV})
	target_link_libraries(RPCameraInterface PRIVATE ${OpenCV_LIBS})
	target_link_libraries(RPCameraInterfaceDemo PRIVATE ${OpenCV_LIBS})
endif()

target_link_libraries(RPCameraInterfaceDemo LINK_PUBLIC RPCameraInterface)


install(FILES include/*.h DESTINATION include/RPCameraInterface)
