#ifndef CAMERAINTERFACE_H
#define CAMERAINTERFACE_H

#include <string>
#include <vector>
#include <memory>

#include "RPCameraInterfaceDefs.h"
#include "ImageFormat.h"
#include "ImageData.h"

namespace RPCameraInterface
{

uint64_t getTimestampMs();

enum class CaptureBackend
{
    Any,
    RPNetworkCamera,
    DShow,
    V4L2,
    LibWebcam,
    OpenCV,
};

class RP_EXPORTS CameraInfo
{
public:
    std::string id;
    std::string name;
    std::string description;
};

class RP_EXPORTS CameraEnumeratorField
{
public:
    std::string name;
    std::string type;
    std::string text;
    std::string value;
    void *extra_param;

    CameraEnumeratorField(std::string name, std::string type, std::string text, std::string value = "");
};

class RP_EXPORTS CameraEnumerator
{
public:
    CameraEnumerator(CaptureBackend backend);
    virtual ~CameraEnumerator();

    virtual bool detectCameras() = 0;
    virtual std::string getCameraId(int index);
    virtual std::string getCameraName(int index);
    virtual std::string getCameraDescription(int index);
    virtual int getCameraIndex(const std::string& id);
    virtual std::string getCameraName(const std::string& id);
    virtual std::string getCameraDescription(const std::string& id);
    virtual int count();

    CaptureBackend backend;
    std::string cameraType;
    std::vector<CameraInfo> listCameras;
    std::vector<CameraEnumeratorField> listRequiredField;//first : field name, second : field type ("text", "number")
};

class RP_EXPORTS CameraInterface
{
public:
    CameraInterface(CaptureBackend backend);
    virtual ~CameraInterface();
    virtual bool open(std::string params) = 0;
    virtual bool close() = 0;
    virtual bool startCapturing() = 0;
    virtual bool stopCapturing() = 0;
    virtual bool hasRecordingCapability();
    virtual bool startRecording();
    virtual bool stopRecordingAndSaveToFile(std::string videoFilename, std::string timestampFilename);
    virtual std::vector<ImageFormat> getAvailableFormats();
    virtual std::vector<VideoCodecType> getAvailableVideoCodec();
    virtual std::vector<VideoContainerType> getAvailableVideoContainer();
    virtual void selectFormat(int formatId);
    virtual void selectVideoContainer(int containerId);
    virtual void selectVideoCodec(int codecId);
    virtual void selectFormat(ImageFormat format);
    virtual void selectVideoContainer(VideoContainerType container);
    virtual void selectVideoCodec(VideoCodecType codec);
    virtual std::shared_ptr<ImageData> getNewFrame(bool skipOldFrames) = 0;
    virtual std::string getErrorMsg();

    CaptureBackend backend;
};

class RP_EXPORTS CameraInterfaceFactory
{
public:
    CameraInterfaceFactory();
    virtual ~CameraInterfaceFactory();
    virtual CameraInterface *createInterface() = 0;
};

class RP_EXPORTS CameraEnumAndFactory
{
public:
    CameraEnumerator* enumerator;
    CameraInterfaceFactory*  interfaceFactory;

    CameraEnumAndFactory(CameraEnumerator* enumerator, CameraInterfaceFactory*  interfaceFactory);
};

RP_EXPORTS std::vector<CaptureBackend> getAvailableCaptureBackends();
RP_EXPORTS std::shared_ptr<CameraEnumerator> getCameraEnumerator(CaptureBackend backend);
RP_EXPORTS std::shared_ptr<CameraInterface> getCameraInterface(CaptureBackend backend);

}//RPCameraInterface

#endif // CAMERAINTERFACE_H
