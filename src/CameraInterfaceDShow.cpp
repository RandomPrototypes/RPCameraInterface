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
    :CameraEnumeratorBase(CaptureBackend::DShow)
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
    :CameraInterfaceBase(CaptureBackend::DShow)
{
    cameraId = -1;
}

CameraInterfaceDShow::~CameraInterfaceDShow()
{
}

bool CameraInterfaceDShow::open(const char *params)
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
size_t CameraInterfaceDShow::getAvailableFormatCount()
{
    return listFormats.size();
}
ImageFormat CameraInterfaceDShow::getAvailableFormat(size_t id)
{
	if(id < listFormats.size())
    	return listFormats[id];
    return ImageFormat();
}
void CameraInterfaceDShow::selectFormat(int formatId)
{
    imageFormat = listFormats[formatId];
}
void CameraInterfaceDShow::selectFormat(ImageFormat format)
{
    imageFormat = format;
}
ImageData *CameraInterfaceDShow::getNewFramePtr(bool skipOldFrames)
{
    videoInput& g_VI = DShowVideoInput::getVideoInput();
    int w = g_VI.getWidth(cameraId), h = g_VI.getHeight(cameraId);
    bool convertRGB = g_VI.getConvertRGB(cameraId);

    ImageData *data = createImageDataRawPtr();
    data->setDataReleasedWhenDestroy(false);
    data->setTimestamp(getTimestampMs());
    /*data->imageFormat.type = ImageType::JPG;
    data->imageFormat.width = imageFormat.width;
    data->imageFormat.height = imageFormat.height;*/
    data->setImageFormat(imageFormat);

	int size;
    data->setDataPtr(g_VI.getRawPixels(cameraId, &size));
    data->setDataSize(size);
    return data;
}
const char *CameraInterfaceDShow::getErrorMsg()
{
    return errorMsg.c_str();
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

}
