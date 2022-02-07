#ifndef CAMERAINTERFACE_H
#define CAMERAINTERFACE_H

#include <string>
#include <vector>
#include <memory>

#include "ImageFormat.h"
#include "ImageData.h"

namespace RPCameraInterface
{

uint64_t getTimestampMs();

class CameraInfo
{
public:
    std::string id;
    std::string name;
    std::string description;
};

class CameraEnumeratorField
{
public:
    std::string name;
    std::string type;
    std::string text;
    std::string value;
    void *extra_param;

    CameraEnumeratorField(std::string name, std::string type, std::string text, std::string value = "");
};

class CameraEnumerator
{
public:
    CameraEnumerator();
    virtual ~CameraEnumerator();

    virtual bool detectCameras() = 0;
    virtual std::string getCameraId(int index);
    virtual std::string getCameraName(int index);
    virtual std::string getCameraDescription(int index);
    virtual int getCameraIndex(const std::string& id);
    virtual std::string getCameraName(const std::string& id);
    virtual std::string getCameraDescription(const std::string& id);
    virtual int count();

    std::string cameraType;
    std::vector<CameraInfo> listCameras;
    std::vector<CameraEnumeratorField> listRequiredField;//first : field name, second : field type ("text", "number")
};

class CameraInterface
{
public:
    CameraInterface();
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
};

class CameraInterfaceFactory
{
public:
    CameraInterfaceFactory();
    virtual ~CameraInterfaceFactory();
    virtual CameraInterface *createInterface() = 0;
};

class CameraEnumAndFactory
{
public:
    CameraEnumerator* enumerator;
    CameraInterfaceFactory*  interfaceFactory;

    CameraEnumAndFactory(CameraEnumerator* enumerator, CameraInterfaceFactory*  interfaceFactory);
};

class CameraMngr
{
public:
    std::vector<CameraEnumAndFactory> listCameraEnumAndFactory;

    void registerEnumAndFactory(CameraEnumerator* enumerator, CameraInterfaceFactory* factory);

    static CameraMngr *getInstance();
private:
    static CameraMngr *instance;

    CameraMngr();
    ~CameraMngr();
};

}//RPCameraInterface

#endif // CAMERAINTERFACE_H
