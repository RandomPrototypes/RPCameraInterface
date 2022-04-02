#ifndef CAMERAINTERFACEBASE_H
#define CAMERAINTERFACEBASE_H

#include "CameraInterface.h"

namespace RPCameraInterface
{

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

    CameraEnumeratorField(const std::string& name, const std::string& type, const std::string& text, const std::string& value = "");
};

class CameraEnumeratorBase : public CameraEnumerator
{
public:
	CameraEnumeratorBase(CaptureBackend backend);
    virtual ~CameraEnumeratorBase();

    virtual bool detectCameras() = 0;
    virtual const char *getCameraId(int index);
    virtual const char *getCameraName(int index);
    virtual const char *getCameraDescription(int index);
    virtual int getCameraIndex(const char *id);
    virtual const char *getCameraName(const char *id);
    virtual const char *getCameraDescription(const char *id);
    virtual int count();
    virtual CaptureBackend getBackend();
    
    CaptureBackend backend;
    std::string cameraType;
    std::vector<CameraInfo> listCameras;
    std::vector<CameraEnumeratorField> listRequiredField;//first : field name, second : field type ("text", "number")
};

class CameraInterfaceBase : public CameraInterface
{
public:
    CameraInterfaceBase(CaptureBackend backend);
    virtual ~CameraInterfaceBase();
    virtual bool open(const char *params) = 0;
    virtual bool close() = 0;
    virtual bool startCapturing() = 0;
    virtual bool stopCapturing() = 0;
    virtual bool hasRecordingCapability();
    virtual bool startRecording();
    virtual bool stopRecordingAndSaveToFile(const char *videoFilename, const char *timestampFilename);
    virtual void selectFormat(int formatId);
    virtual void selectVideoContainer(int containerId);
    virtual void selectVideoCodec(int codecId);
    virtual void selectFormat(ImageFormat format);
    virtual void selectVideoContainer(VideoContainerType container);
    virtual void selectVideoCodec(VideoCodecType codec);
    virtual const char *getErrorMsg();
    virtual CaptureBackend getBackend();

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);
	virtual size_t getAvailableVideoCodecCount();
	virtual VideoCodecType getAvailableVideoCodec(size_t id);
    virtual size_t getAvailableVideoContainerCount();
    virtual VideoContainerType getAvailableVideoContainer(size_t id);
    
    CaptureBackend backend;
};

}//RPCameraInterface

#endif // CAMERAINTERFACE_H
