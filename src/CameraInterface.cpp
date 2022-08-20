#include <RPCameraInterface/CameraInterface.h>
#include <sstream>
#include <chrono>
#include <string.h>

#include "RPCameraInterface/CameraInterfaceRPNetwork.h"

#ifdef HAVE_DSHOW
#include "RPCameraInterface/CameraInterfaceDShow.h"
#endif

#ifdef HAVE_V4L2
#include "RPCameraInterface/CameraInterfaceV4L2.h"
#endif

#include "RPCameraInterface/CameraInterfaceOpenCV.h"

#ifdef USE_DEPTHAI
#include "RPCameraInterface/CameraInterfaceDepthAI.h"
#endif

namespace RPCameraInterface
{

uint64_t getTimestampMs()
{
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count();
}

CameraInterface::CameraInterface()
{
}

CameraInterface::~CameraInterface()
{
}

CameraEnumeratorField::CameraEnumeratorField()
{
}
CameraEnumeratorField::~CameraEnumeratorField()
{
}

CameraEnumerator::CameraEnumerator()
{
}

CameraEnumerator::~CameraEnumerator()
{
}

ImageData *CameraInterface::getNewFramePtr(bool skipOldFrames)
{
	return NULL;
}
size_t CameraInterface::getAvailableFormatCount()
{
	return 0;
}
ImageFormat CameraInterface::getAvailableFormat(size_t id)
{
	return ImageFormat();
}
size_t CameraInterface::getAvailableVideoCodecCount()
{
	return 0;
}
VideoCodecType CameraInterface::getAvailableVideoCodec(size_t id)
{
	return VideoCodecType::UNKNOWN;
}
size_t CameraInterface::getAvailableVideoContainerCount()
{
	return 0;
}
VideoContainerType CameraInterface::getAvailableVideoContainer(size_t id)
{
	return VideoContainerType::UNKNOWN;
}

RPCAM_EXPORTS std::vector<CaptureBackend> getAvailableCaptureBackends()
{
    std::vector<CaptureBackend> list;
    
    #ifdef HAVE_DSHOW
    list.push_back(CaptureBackend::DShow);
    #endif
    
    #ifdef HAVE_V4L2
    list.push_back(CaptureBackend::V4L2);
    #endif

	list.push_back(CaptureBackend::RPNetworkCamera);

    list.push_back(CaptureBackend::OpenCV);

    #ifdef USE_DEPTHAI
    list.push_back(CaptureBackend::DepthAI);
    #endif

    return list;
}
RPCAM_EXPORTS std::shared_ptr<CameraEnumerator> getCameraEnumerator(CaptureBackend backend)
{
	if(backend == CaptureBackend::Any) {
		std::vector<CaptureBackend> list = getAvailableCaptureBackends();
		if(list.size() > 0)
			backend = list[0];
	}
    switch(backend)
    {
        #ifdef HAVE_DSHOW
        case CaptureBackend::DShow:
            return std::make_shared<CameraEnumeratorDShow>();
        #endif
        
        #ifdef HAVE_V4L2
    	case CaptureBackend::V4L2:
            return std::make_shared<CameraEnumeratorV4L2>();
    	#endif
    	
    	case CaptureBackend::RPNetworkCamera:
            return std::make_shared<CameraEnumeratorRPNetwork>();
        
        case CaptureBackend::OpenCV:
            return std::make_shared<CameraEnumeratorOpenCV>();

        #ifdef USE_DEPTHAI
        case CaptureBackend::DepthAI:
            return std::make_shared<CameraEnumeratorDepthAI>();
        #endif

        default:
            return std::shared_ptr<CameraEnumerator>();
    }

    return std::shared_ptr<CameraEnumerator>();
}
RPCAM_EXPORTS std::shared_ptr<CameraInterface> getCameraInterface(CaptureBackend backend)
{
	if(backend == CaptureBackend::Any) {
		std::vector<CaptureBackend> list = getAvailableCaptureBackends();
		if(list.size() > 0)
			backend = list[0];
	}
    switch(backend)
    {
        #ifdef HAVE_DSHOW
        case CaptureBackend::DShow:
            return std::make_shared<CameraInterfaceDShow>();
        #endif
        
        #ifdef HAVE_V4L2
        case CaptureBackend::V4L2:
            return std::make_shared<CameraInterfaceV4L2>();
        #endif
        
    	case CaptureBackend::RPNetworkCamera:
            return std::make_shared<CameraInterfaceRPNetwork>();
        
        case CaptureBackend::OpenCV:
            return std::make_shared<CameraInterfaceOpenCV>();
        
        #ifdef USE_DEPTHAI
        case CaptureBackend::DepthAI:
            return std::make_shared<CameraInterfaceDepthAI>();
        #endif
        default:
            return std::shared_ptr<CameraInterface>();
    }

    return std::shared_ptr<CameraInterface>();
}

}
