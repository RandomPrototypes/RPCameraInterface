#ifndef CAMERAINTERFACERPNETWORK_H
#define CAMERAINTERFACERPNETWORK_H

#include <RPCameraInterface/CameraInterfaceBase.h>
#include <BufferedSocket/BufferedSocket.h>
#include <sstream>

namespace RPCameraInterface
{

class CameraEnumeratorRPNetwork : public CameraEnumeratorBase
{
public:
    CameraEnumeratorRPNetwork();
    virtual ~CameraEnumeratorRPNetwork();

    virtual bool detectCameras();
};

class CameraInterfaceRPNetwork : public CameraInterfaceBase
{
public:
    CameraInterfaceRPNetwork();
    ~CameraInterfaceRPNetwork();
    virtual bool open(const char *params);
    virtual bool close();
    virtual void selectFormat(ImageFormat format);
    virtual void selectVideoContainer(VideoContainerType container);
    virtual void selectVideoCodec(VideoCodecType codec);
    virtual const char *getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();
    virtual bool hasRecordingCapability();
    virtual bool startRecording();
    virtual bool stopRecordingAndSaveToFile(std::string videoFilename, std::string timestampFilename);

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);
	virtual size_t getAvailableVideoCodecCount();
	virtual VideoCodecType getAvailableVideoCodec(size_t id);
    virtual size_t getAvailableVideoContainerCount();
    virtual VideoContainerType getAvailableVideoContainer(size_t id);

private:
    bool syncTimestamp();

    int deviceId;
    std::vector<ImageFormat> listFormats;
    ImageFormat imageFormat;
    VideoCodecType videoCodecType;
    VideoContainerType videoContainerType;
    std::shared_ptr<BufferedSocket> bufferedSock;
    std::string errorMsg;
    int64_t timestampOffsetMs;
};

}

#endif // CAMERAINTERFACEANDROID_H
