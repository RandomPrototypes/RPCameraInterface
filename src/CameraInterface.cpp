#include "CameraInterface.h"
#include <sstream>
#include <qdebug.h>

namespace RPCameraInterface
{

CameraMngr* CameraMngr::instance = NULL;

uint64_t getTimestampMs()
{
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime).count();
}

std::string toString(ImageType type)
{
    switch(type)
    {
        case ImageType::UNKNOWN:
            return "unknown";
        case ImageType::YUYV422:
            return "yuyv422";
        case ImageType::RGB:
            return "rgb";
        case ImageType::BGR:
            return "bgr";
        case ImageType::MJPG:
            return "mjpg";
        default:
            return "unknown";
    }
}

CameraInterface::CameraInterface()
{

}

CameraInterface::~CameraInterface()
{

}

std::vector<ImageFormat> CameraInterface::getAvailableFormats()
{
    return std::vector<ImageFormat>();
}

std::vector<VideoContainerType> CameraInterface::getAvailableVideoContainer()
{
    return std::vector<VideoContainerType>();
}

std::vector<VideoCodecType> CameraInterface::getAvailableVideoCodec()
{
    return std::vector<VideoCodecType>();
}

void CameraInterface::selectFormat(int formatId)
{
    std::vector<ImageFormat> listFormats = getAvailableFormats();
    if(formatId >= 0 && formatId < listFormats.size())
        selectFormat(listFormats[formatId]);
}

void CameraInterface::selectVideoContainer(int containerId)
{
    std::vector<VideoContainerType> listContainers = getAvailableVideoContainer();
    if(containerId >= 0 && containerId < listContainers.size())
        selectVideoContainer(listContainers[containerId]);
}


void CameraInterface::selectVideoCodec(int codecId)
{
    std::vector<VideoCodecType> listCodecs = getAvailableVideoCodec();
    if(codecId >= 0 && codecId < listCodecs.size())
        selectVideoCodec(listCodecs[codecId]);
}

void CameraInterface::selectFormat(ImageFormat format)
{

}

void CameraInterface::selectVideoContainer(VideoContainerType container)
{

}

void CameraInterface::selectVideoCodec(VideoCodecType codec)
{

}

bool CameraInterface::hasRecordingCapability()
{
    return false;
}
bool CameraInterface::startRecording()
{
    return false;
}
bool CameraInterface::stopRecordingAndSaveToFile(std::string videoFilename, std::string timestampFilename)
{
    return false;
}

std::string CameraInterface::getErrorMsg()
{
    return "";
}

CameraInterfaceFactory::CameraInterfaceFactory()
{
}

CameraInterfaceFactory::~CameraInterfaceFactory()
{
}

CameraEnumeratorField::CameraEnumeratorField(std::string name, std::string type, std::string text, std::string value)
    :name(name), type(type), text(text), value(value)
{

}

CameraEnumerator::CameraEnumerator()
{
}

CameraEnumerator::~CameraEnumerator()
{
}

std::string CameraEnumerator::getCameraId(int id)
{
    if(id >= 0 && id < listCameras.size())
        return listCameras[id].id;
    return "unknown";
}

std::string CameraEnumerator::getCameraName(int id)
{
    if(id >= 0 && id < listCameras.size())
        return listCameras[id].name;
    return "unknown";
}

std::string CameraEnumerator::getCameraDescription(int id)
{
    if(id >= 0 && id < listCameras.size())
        return listCameras[id].description;
    return "unknown";
}

int CameraEnumerator::count()
{
    return listCameras.size();
}

int CameraEnumerator::getCameraIndex(const std::string& id)
{
    for(size_t i = 0; i < listCameras.size(); i++)
        if(listCameras[i].id == id)
            return i;
    return -1;
}

std::string CameraEnumerator::getCameraName(const std::string& id)
{
    int index = getCameraIndex(id);
    if(index >= 0)
        return getCameraName(index);
    return "unknown";
}
std::string CameraEnumerator::getCameraDescription(const std::string& id)
{
    int index = getCameraIndex(id);
    if(index >= 0)
        return getCameraDescription(index);
    return "unknown";
}

CameraEnumAndFactory::CameraEnumAndFactory(CameraEnumerator* enumerator, CameraInterfaceFactory*  interfaceFactory)
    :enumerator(enumerator), interfaceFactory(interfaceFactory)
{

}

CameraMngr::CameraMngr()
{

}

CameraMngr::~CameraMngr()
{
    for(size_t i = 0; i < listCameraEnumAndFactory.size(); i++)
    {
        delete listCameraEnumAndFactory[i].enumerator;
        delete listCameraEnumAndFactory[i].interfaceFactory;
    }
}

void CameraMngr::registerEnumAndFactory(CameraEnumerator* enumerator, CameraInterfaceFactory* factory)
{
    listCameraEnumAndFactory.push_back(CameraEnumAndFactory(enumerator, factory));
}

CameraMngr *CameraMngr::getInstance()
{
    if(instance == NULL)
        instance = new CameraMngr();
    return instance;
}

}
