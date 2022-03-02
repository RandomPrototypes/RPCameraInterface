#include <RPCameraInterface/CameraInterface.h>
#include <sstream>
#include <chrono>

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
        case ImageType::GRAY8:
            return "GRAY8";
        case ImageType::RGB24:
            return "RGB24";
        case ImageType::BGR24:
            return "BGR24";
        case ImageType::BGRA32:
            return "BGRA32";
        case ImageType::RGB555:
            return "RGB555";
        case ImageType::RGB565:
            return "RGB565";
        case ImageType::AYUV:
            return "AYUV";
        case ImageType::YUV420P:
            return "YUV420P";
        case ImageType::UYVY:
            return "UYVY";
        case ImageType::Y211:
            return "Y211";
        case ImageType::Y41P:
            return "Y41P";
        case ImageType::YUYV422:
            return "YUYV422";
        case ImageType::YVU9:
            return "YVU9";
        case ImageType::YVYU:
            return "YVYU";
        case ImageType::JPG:
            return "JPG";
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
