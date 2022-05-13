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

namespace RPCameraInterface
{

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

CameraInterfaceBase::CameraInterfaceBase(CaptureBackend backend)
    :CameraInterface(), backend(backend)
{

}

CameraInterfaceBase::~CameraInterfaceBase()
{

}

void CameraInterfaceBase::selectFormat(int formatId)
{
    if(formatId >= 0 && formatId < getAvailableFormatCount())
        selectFormat(getAvailableFormat(formatId));
}

void CameraInterfaceBase::selectVideoContainer(int containerId)
{
    if(containerId >= 0 && containerId < getAvailableVideoContainerCount())
        selectVideoContainer(getAvailableVideoContainer(containerId));
}


void CameraInterfaceBase::selectVideoCodec(int codecId)
{
    if(codecId >= 0 && codecId < getAvailableVideoCodecCount())
        selectVideoCodec(getAvailableVideoCodec(codecId));
}

void CameraInterfaceBase::selectFormat(ImageFormat format)
{

}

void CameraInterfaceBase::selectVideoContainer(VideoContainerType container)
{

}

void CameraInterfaceBase::selectVideoCodec(VideoCodecType codec)
{

}

bool CameraInterfaceBase::hasRecordingCapability()
{
    return false;
}
bool CameraInterfaceBase::startRecording()
{
    return false;
}
bool CameraInterfaceBase::stopRecordingAndSaveToFile(const char *videoFilename, const char *timestampFilename)
{
    return false;
}

const char *CameraInterfaceBase::getErrorMsg()
{
    return "";
}

CaptureBackend CameraInterfaceBase::getBackend()
{
	return backend;
}

ImageData *CameraInterfaceBase::getNewFramePtr(bool skipOldFrames)
{
	return NULL;
}
size_t CameraInterfaceBase::getAvailableFormatCount()
{
	return 0;
}
ImageFormat CameraInterfaceBase::getAvailableFormat(size_t id)
{
	return ImageFormat();
}
size_t CameraInterfaceBase::getAvailableVideoCodecCount()
{
	return 0;
}
VideoCodecType CameraInterfaceBase::getAvailableVideoCodec(size_t id)
{
	return VideoCodecType::UNKNOWN;
}
size_t CameraInterfaceBase::getAvailableVideoContainerCount()
{
	return 0;
}
VideoContainerType CameraInterfaceBase::getAvailableVideoContainer(size_t id)
{
	return VideoContainerType::UNKNOWN;
}

CameraEnumeratorFieldBase::CameraEnumeratorFieldBase(const std::string& name, const std::string& type, const std::string& text, const std::string& value)
    :name(name), type(type), text(text), value(value), extraParam(NULL)
{
}

CameraEnumeratorFieldBase::~CameraEnumeratorFieldBase()
{
}

const char *CameraEnumeratorFieldBase::getName()
{
    return name.c_str();
}
const char *CameraEnumeratorFieldBase::getType()
{
    return type.c_str();
}
const char *CameraEnumeratorFieldBase::getText()
{
    return text.c_str();
}
const char *CameraEnumeratorFieldBase::getValue()
{
    return value.c_str();
}
void *CameraEnumeratorFieldBase::getExtraParam()
{
    return extraParam;
}

void CameraEnumeratorFieldBase::setName(const char *name)
{
    this->name = name;
}
void CameraEnumeratorFieldBase::setType(const char *type)
{
    this->type = type;
}
void CameraEnumeratorFieldBase::setText(const char *text)
{
    this->text = text;
}
void CameraEnumeratorFieldBase::setValue(const char *value)
{
    this->value = value;
}
void CameraEnumeratorFieldBase::setExtraParam(void *param)
{
    this->extraParam = param;
}

CameraEnumeratorBase::CameraEnumeratorBase(CaptureBackend backend)
    :CameraEnumerator(), backend(backend)
{
}

CameraEnumeratorBase::~CameraEnumeratorBase()
{
    for(size_t i = 0; i < listParamField.size(); i++)
        delete listParamField[i];
}

const char *CameraEnumeratorBase::getCameraId(int id)
{
    if(id >= 0 && id < listCameras.size())
        return listCameras[id].id.c_str();
    return "unknown";
}

const char *CameraEnumeratorBase::getCameraName(int id)
{
    if(id >= 0 && id < listCameras.size())
        return listCameras[id].name.c_str();
    return "unknown";
}

const char *CameraEnumeratorBase::getCameraDescription(int id)
{
    if(id >= 0 && id < listCameras.size())
        return listCameras[id].description.c_str();
    return "unknown";
}

int CameraEnumeratorBase::count()
{
    return listCameras.size();
}

int CameraEnumeratorBase::getCameraIndex(const char *id)
{
    for(size_t i = 0; i < listCameras.size(); i++)
        if(!strncmp(listCameras[i].id.c_str(), id, listCameras[i].id.size()))
            return i;
    return -1;
}

const char *CameraEnumeratorBase::getCameraName(const char *id)
{
    int index = getCameraIndex(id);
    if(index >= 0)
        return getCameraName(index);
    return "unknown";
}
const char *CameraEnumeratorBase::getCameraDescription(const char *id)
{
    int index = getCameraIndex(id);
    if(index >= 0)
        return getCameraDescription(index);
    return "unknown";
}

CaptureBackend CameraEnumeratorBase::getBackend()
{
	return backend;
}

const char *CameraEnumeratorBase::getCameraType()
{
	return cameraType.c_str();
}

int CameraEnumeratorBase::getNbParamField()
{
    return listParamField.size();
}
CameraEnumeratorField *CameraEnumeratorBase::getParamField(int id)
{
    return listParamField[id];
}

}
