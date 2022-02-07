#ifndef CAMERAINTERFACEANDROID_H
#define CAMERAINTERFACEANDROID_H

#include "CameraInterface.h"
#include "BufferedSocket.h"
#include <sstream>

namespace RPCameraInterface
{

class CameraEnumeratorAndroid : public CameraEnumerator
{
public:
    CameraEnumeratorAndroid();
    virtual ~CameraEnumeratorAndroid();

    virtual bool detectCameras();
};

class CameraInterfaceAndroid : public CameraInterface
{
public:
    CameraInterfaceAndroid();
    ~CameraInterfaceAndroid();
    virtual bool open(std::string params);
    virtual bool close();
    virtual std::vector<ImageFormat> getAvailableFormats();
    virtual std::vector<VideoCodecType> getAvailableVideoCodec();
    virtual std::vector<VideoContainerType> getAvailableVideoContainer();
    virtual void selectFormat(ImageFormat format);
    virtual void selectVideoContainer(VideoContainerType container);
    virtual void selectVideoCodec(VideoCodecType codec);
    virtual std::shared_ptr<ImageData> getNewFrame(bool skipOldFrames);
    virtual std::string getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();
    virtual bool hasRecordingCapability();
    virtual bool startRecording();
    virtual bool stopRecordingAndSaveToFile(std::string videoFilename, std::string timestampFilename);
private:
    bool syncTimestamp();

    int deviceId;
    std::vector<ImageFormat> listFormats;
    ImageFormat imageFormat;
    VideoCodecType videoCodecType;
    VideoContainerType videoContainerType;
    BufferedSocket bufferedSock;
    std::string errorMsg;
    int64_t timestampOffsetMs;
};

class CameraInterfaceFactoryAndroid : public CameraInterfaceFactory
{
public:
    CameraInterfaceFactoryAndroid();
    virtual ~CameraInterfaceFactoryAndroid();
    virtual CameraInterface *createInterface();
};

}

#endif // CAMERAINTERFACEANDROID_H
