#include <RPCameraInterface/CameraInterfaceDepthAI.h>

namespace RPCameraInterface
{

CameraEnumeratorDepthAI::CameraEnumeratorDepthAI()
    :CameraEnumeratorBase(CaptureBackend::DepthAI)
{
    cameraType = "DepthAI camera";
}

CameraEnumeratorDepthAI::~CameraEnumeratorDepthAI()
{
}

bool CameraEnumeratorDepthAI::detectCameras()
{
    listCameras.clear();
    dai::DeviceInfo info;
    auto deviceInfos = dai::DeviceBase::getAllAvailableDevices();
    for(int i = 0; i < deviceInfos.size(); i++) {
        const auto& devInfo = deviceInfos[i];
        CameraInfo camInfo;
        camInfo.id = devInfo.getMxId();
        camInfo.name = devInfo.toString();
        camInfo.description = devInfo.toString();
        listCameras.push_back(camInfo);
    }
    return true;
}

CameraInterfaceDepthAI::CameraInterfaceDepthAI()
    :CameraInterfaceBase(CaptureBackend::DepthAI)
{
    device = NULL;
}

CameraInterfaceDepthAI::~CameraInterfaceDepthAI()
{
}

bool CameraInterfaceDepthAI::open(const char *params)
{
    std::tuple<bool, dai::DeviceInfo> deviceInfoTmp = dai::DeviceBase::getDeviceByMxId(params);
    if(!std::get<0>(deviceInfoTmp))
        return false;
    deviceInfo = std::get<1>(deviceInfoTmp);
    
    if(device != NULL)
        close();
    device = new dai::Device(pipeline, deviceInfo);

    std::unordered_map<dai::CameraBoardSocket, std::string> camSensorNames = device->getCameraSensorNames();
    for(const std::pair<dai::CameraBoardSocket, std::string>& camSensorName : camSensorNames) {
        printf("%s\n", camSensorName.second.c_str());
    }
    detectAvailableFormats();
    
    return true;
}

bool CameraInterfaceDepthAI::close()
{
    if(device != NULL)
        delete device;
    device = NULL;
    return true;
}

bool dai2size(dai::ColorCameraProperties::SensorResolution res, int *w, int *h) {
    switch(res) {
        case dai::ColorCameraProperties::SensorResolution::THE_13_MP:
            *w = 4208;
            *h = 3120;
            return true;
        case dai::ColorCameraProperties::SensorResolution::THE_12_MP:
            *w = 4056;
            *h = 3040;
            return true;
        case dai::ColorCameraProperties::SensorResolution::THE_4_K:
            *w = 3840;
            *h = 2160;
            return true;
        case dai::ColorCameraProperties::SensorResolution::THE_1080_P:
            *w = 1920;
            *h = 1080;
            return true;
        case dai::ColorCameraProperties::SensorResolution::THE_800_P:
            *w = 1280;
            *h = 800;
            return true;
        case dai::ColorCameraProperties::SensorResolution::THE_720_P:
            *w = 1280;
            *h = 720;
            return true;
        default:
            return false;
    }
}

bool dai2size(dai::MonoCameraProperties::SensorResolution res, int *w, int *h) {
    switch(res) {
        case dai::MonoCameraProperties::SensorResolution::THE_800_P:
            *w = 1280;
            *h = 800;
            return true;
        case dai::MonoCameraProperties::SensorResolution::THE_720_P:
            *w = 1280;
            *h = 720;
            return true;
        case dai::MonoCameraProperties::SensorResolution::THE_480_P:
            *w = 640;
            *h = 480;
            return true;
        case dai::MonoCameraProperties::SensorResolution::THE_400_P:
            *w = 640;
            *h = 400;
            return true;
        default:
            return false;
    }
}

bool size2dai(int w, int h, dai::ColorCameraProperties::SensorResolution *res) {
    if(w == 4208 && h == 3120) {
        *res = dai::ColorCameraProperties::SensorResolution::THE_13_MP;
        return true;
    } else if(w == 4056 && h == 3040) {
        *res = dai::ColorCameraProperties::SensorResolution::THE_12_MP;
        return true;
    } else if(w == 3840 && h == 2160) {
        *res = dai::ColorCameraProperties::SensorResolution::THE_4_K;
        return true;
    } else if(w == 1920 && h == 1080) {
        *res = dai::ColorCameraProperties::SensorResolution::THE_1080_P;
        return true;
    } else if(w == 1280 && h == 800) {
        *res = dai::ColorCameraProperties::SensorResolution::THE_800_P;
        return true;
    } else if(w == 1280 && h == 720) {
        *res = dai::ColorCameraProperties::SensorResolution::THE_720_P;
        return true;
    } else {
        return false;
    }
}

bool size2dai(int w, int h, dai::MonoCameraProperties::SensorResolution *res) {
    if(w == 1280 && h == 800) {
        *res = dai::MonoCameraProperties::SensorResolution::THE_800_P;
        return true;
    } else if(w == 1280 && h == 720) {
        *res = dai::MonoCameraProperties::SensorResolution::THE_720_P;
        return true;
    } else if(w == 640 && h == 480) {
        *res = dai::MonoCameraProperties::SensorResolution::THE_480_P;
        return true;
    } else if(w == 640 && h == 400) {
        *res = dai::MonoCameraProperties::SensorResolution::THE_400_P;
        return true;
    } else {
        return false;
    }
}

void CameraInterfaceDepthAI::detectAvailableFormats()
{
    listFormats.clear();
    std::vector<dai::ColorCameraProperties::SensorResolution> resolutions = {dai::ColorCameraProperties::SensorResolution::THE_720_P,
                                                                             dai::ColorCameraProperties::SensorResolution::THE_800_P,
                                                                             dai::ColorCameraProperties::SensorResolution::THE_1080_P,
                                                                             dai::ColorCameraProperties::SensorResolution::THE_4_K};
    for(int compression = 0; compression < 2; compression++) {
        for(dai::ColorCameraProperties::SensorResolution resolution : resolutions) {
            ImageFormat format;
            format.type = compression == 0 ? ImageType::BGR24 : ImageType::JPG;
            int w, h;
            if(!dai2size(resolution, &w, &h))
                continue;
            format.width = w;
            format.height = h;
            listFormats.push_back(format);
        }
    }
}

size_t CameraInterfaceDepthAI::getAvailableFormatCount()
{
	return listFormats.size();
}

ImageFormat CameraInterfaceDepthAI::getAvailableFormat(size_t id)
{
	if(id < listFormats.size())
		return listFormats[id];
	return ImageFormat();
}
	
void CameraInterfaceDepthAI::selectFormat(int formatId)
{
    if(formatId < listFormats.size())
        imageFormat = listFormats[formatId];
}
void CameraInterfaceDepthAI::selectFormat(ImageFormat format)
{
    imageFormat = format;
}
ImageData *CameraInterfaceDepthAI::getNewFramePtr(bool skipOldFrames)
{
    videoIn = video->get<dai::ImgFrame>();

    ImageData *data = createImageDataRawPtr();
    data->setDataReleasedWhenDestroy(false);
    data->setTimestamp(getTimestampMs());
    data->getImageFormat().type = imageFormat.type;
    data->getImageFormat().width = imageFormat.width;
    data->getImageFormat().height = imageFormat.height;
    if(imageFormat.type == ImageType::JPG) {
        std::vector<uint8_t>& imgData = videoIn->getData();
        //cv::Mat img = cv::imdecode(cv::Mat(imgData), cv::IMREAD_COLOR);
        //data->setDataPtr(const_cast<unsigned char*>(img.ptr<unsigned char>(0)));
        //data->setDataSize(imageFormat.height*imageFormat.width*3);
        data->setDataPtr(const_cast<unsigned char*>(&imgData[0]));
        data->setDataSize(imgData.size());
    } else {
        img = videoIn->getCvFrame();
        data->setDataPtr(const_cast<unsigned char*>(img.ptr<unsigned char>(0)));
        data->setDataSize(imageFormat.height*imageFormat.width*3);
    }
    return data;
}
const char *CameraInterfaceDepthAI::getErrorMsg()
{
    return errorMsg.c_str();
}

bool CameraInterfaceDepthAI::startCapturing()
{
    close();
    dai::ColorCameraProperties::SensorResolution resolution;
    if(!size2dai(imageFormat.width, imageFormat.height, &resolution))
        return false;
    camRgb = pipeline.create<dai::node::ColorCamera>();
    xoutVideo = pipeline.create<dai::node::XLinkOut>();

    if(imageFormat.type == ImageType::JPG)
        videnc = pipeline.create<dai::node::VideoEncoder>();

    xoutVideo->setStreamName("video");

    camRgb->setBoardSocket(dai::CameraBoardSocket::RGB);
    camRgb->setResolution(resolution);
    camRgb->setVideoSize(imageFormat.width, imageFormat.height);

    xoutVideo->input.setBlocking(false);
    xoutVideo->input.setQueueSize(1);

    if(imageFormat.type == ImageType::JPG) {
        videnc->setDefaultProfilePreset(30, dai::VideoEncoderProperties::Profile::MJPEG);
        camRgb->video.link(videnc->input);
        videnc->bitstream.link(xoutVideo->input);
    } else {
        camRgb->video.link(xoutVideo->input);
    }

    device = new dai::Device(pipeline, deviceInfo);

    video = device->getOutputQueue("video");

    return true;
}
bool CameraInterfaceDepthAI::stopCapturing()
{
    camRgb = NULL;
    xoutVideo = NULL;
    video = NULL;
    videnc = NULL;
    videoIn = NULL;
    if(device != NULL)
        delete device;
    device = NULL;
    return true;
}

}
