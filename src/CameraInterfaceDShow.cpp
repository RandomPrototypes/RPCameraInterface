#include <RPCameraInterface/CameraInterfaceDShow.h>
#include "cap_dshow/cap_dshow.hpp"

namespace RPCameraInterface
{

DShowVideoInput *DShowVideoInput::instance = NULL;

DShowVideoInput::DShowVideoInput()
{
    CoInitialize(0);
    g_VI = new videoInput();
}
DShowVideoInput::~DShowVideoInput()
{
    delete g_VI;
    CoUninitialize();
}
videoInput& DShowVideoInput::getVideoInput()
{
    if(instance == NULL)
        instance = new DShowVideoInput();
    return *(instance->g_VI);
}

CameraEnumeratorDShow::CameraEnumeratorDShow()
    :CameraEnumerator(CaptureBackend::DShow)
{
    cameraType = "USB camera";
}

CameraEnumeratorDShow::~CameraEnumeratorDShow()
{
}

bool CameraEnumeratorDShow::detectCameras()
{
    videoInput& g_VI = DShowVideoInput::getVideoInput();
    int numDevices = g_VI.listDevices();

    listCameras.clear();
    for(int i = 0; i < numDevices; i++) {
        CameraInfo cam;
        cam.id = std::to_string(i);
        cam.name = g_VI.getDeviceName(i);
        cam.description = cam.name;
        listCameras.push_back(cam);
    }
    return true;
}

CameraInterfaceDShow::CameraInterfaceDShow()
    :CameraInterface(CaptureBackend::DShow)
{
    cameraId = -1;
}

CameraInterfaceDShow::~CameraInterfaceDShow()
{
}

ImageType toImageType(const std::string& guid)
{

    if(guid == "AYUV") {
        return ImageType::AYUV;
    } else if(guid == "RGB24") {
        return ImageType::RGB24;
    } else if(guid == "RGB32") {
        return ImageType::BGRA32;
    } else if(guid == "RGB555") {
        return ImageType::RGB555;
    } else if(guid == "RGB565") {
        return ImageType::RGB565;
    } else if(guid == "IYUV" || guid == "I420" || guid == "YV12") {
        return ImageType::YUV420P;
    } else if(guid == "UYVY") {
        return ImageType::UYVY;
    } else if(guid == "Y211") {
        return ImageType::Y211;
    } else if(guid == "Y411" || guid == "Y41P") {
        return ImageType::Y41P;
    } else if(guid == "YUY2" || guid == "YUYV") {
        return ImageType::YUYV422;
    } else if(guid == "YVU9") {
        return ImageType::YVU9;
    } else if(guid == "YVYU") {
        return ImageType::YVYU;
    } else if(guid == "Y8" || guid == "Y800" || guid == "GREY") {
        return ImageType::GRAY8;
    } else if(guid == "BY8") {
        return ImageType::BY8;
    } else if(guid == "Y16") {
        return ImageType::Y16;
    } else if(guid == "MJPG") {
        return ImageType::JPG;
    } else {
        return ImageType::UNKNOWN;
    }
}

bool CameraInterfaceDShow::open(std::string params)
{
    videoInput& g_VI = DShowVideoInput::getVideoInput();
    try {
        cameraId = std::stoi(params);

        std::vector<videoFormat> listNativeFormats = g_VI.getVideoFormats(cameraId);

        listFormats.clear();
        listFormatsFourcc.clear();
        for(videoFormat &format : listNativeFormats) {
            ImageFormat resultFormat;
            resultFormat.type = toImageType(format.format);
            resultFormat.width = format.width;
            resultFormat.height = format.height;
            if(resultFormat.type != ImageType::UNKNOWN) {
                listFormats.push_back(resultFormat);
                listFormatsFourcc.push_back(format.fourcc);
            }
        }
    } catch (std::invalid_argument const& ex) {
        errorMsg = std::string("can not convert to number: ")+params;
        return false;
    }
    return true;
}
bool CameraInterfaceDShow::close()
{
    DShowVideoInput::getVideoInput().stopDevice(cameraId);
    cameraId = -1;
    return true;
}
std::vector<ImageFormat> CameraInterfaceDShow::getAvailableFormats()
{
    return listFormats;
}
void CameraInterfaceDShow::selectFormat(int formatId)
{
    imageFormat = listFormats[formatId];
}
void CameraInterfaceDShow::selectFormat(ImageFormat format)
{
    imageFormat = format;
}
std::shared_ptr<ImageData> CameraInterfaceDShow::getNewFrame(bool skipOldFrames)
{
    videoInput& g_VI = DShowVideoInput::getVideoInput();
    int w = g_VI.getWidth(cameraId), h = g_VI.getHeight(cameraId);
    bool convertRGB = g_VI.getConvertRGB(cameraId);

    std::shared_ptr<ImageData> data = std::make_shared<ImageData>();
    data->releaseDataWhenDestroy = false;
    data->timestamp = getTimestampMs();
    data->imageFormat.type = ImageType::JPG;
    data->imageFormat.width = imageFormat.width;
    data->imageFormat.height = imageFormat.height;

    data->data = g_VI.getRawPixels(cameraId, &(data->dataSize));
    return data;
}
std::string CameraInterfaceDShow::getErrorMsg()
{
    return errorMsg;
}

bool CameraInterfaceDShow::startCapturing()
{
    videoInput& g_VI = DShowVideoInput::getVideoInput();

    bool found = false;
    int fourcc;
    for(size_t i = 0; i < listFormats.size(); i++) {
        if(listFormats[i].type == imageFormat.type) {
            fourcc = listFormatsFourcc[i];
            found = true;
        }
    }
    if(!found)
        return false;
    g_VI.setupDeviceFourcc(cameraId, imageFormat.width, imageFormat.height, fourcc);

    return g_VI.isDeviceSetup(cameraId);
}
bool CameraInterfaceDShow::stopCapturing()
{
    /*try
    {
        if(device != NULL)
        {
            device->close();
            delete device;
            device = NULL;
            return true;
        }
    } catch(const webcam::webcam_exception & exc_) {
        errorMsg = std::string("startCapturing : ")+exc_.what();
        return false;
    }*/

    return false;
}

CameraInterfaceFactoryDShow::CameraInterfaceFactoryDShow()
{
}

CameraInterfaceFactoryDShow::~CameraInterfaceFactoryDShow()
{
}

CameraInterface *CameraInterfaceFactoryDShow::createInterface()
{
    return new CameraInterfaceDShow();
}

}
