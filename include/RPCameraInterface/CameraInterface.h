#pragma once

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

class RPCAM_EXPORTS CameraEnumerator
{
public:
    CameraEnumerator();
    virtual ~CameraEnumerator();

    virtual bool detectCameras() = 0;
    virtual const char *getCameraId(int index) = 0;
    virtual const char *getCameraName(int index) = 0;
    virtual const char *getCameraDescription(int index) = 0;
    virtual int getCameraIndex(const char *id) = 0;
    virtual const char *getCameraName(const char *id) = 0;
    virtual const char *getCameraDescription(const char *id) = 0;
    virtual int count() = 0;
    virtual CaptureBackend getBackend() = 0;
};

class RPCAM_EXPORTS CameraInterface
{
public:
    CameraInterface();
    virtual ~CameraInterface();
    virtual bool open(const char *params) = 0;
    virtual bool close() = 0;
    virtual bool startCapturing() = 0;
    virtual bool stopCapturing() = 0;
    virtual bool hasRecordingCapability() = 0;
    virtual bool startRecording() = 0;
    virtual bool stopRecordingAndSaveToFile(const char *videoFilename, const char *timestampFilename) = 0;
    virtual void selectFormat(int formatId) = 0;
    virtual void selectVideoContainer(int containerId) = 0;
    virtual void selectVideoCodec(int codecId) = 0;
    virtual void selectFormat(ImageFormat format) = 0;
    virtual void selectVideoContainer(VideoContainerType container) = 0;
    virtual void selectVideoCodec(VideoCodecType codec) = 0;
    virtual const char *getErrorMsg() = 0;
    virtual CaptureBackend getBackend() = 0;
    
    std::shared_ptr<ImageData> getNewFrame(bool skipOldFrames);
    std::vector<ImageFormat> getListAvailableFormat();
    std::vector<VideoCodecType> getListAvailableVideoCodec();
    std::vector<VideoContainerType> getListAvailableVideoContainer();

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);
	virtual size_t getAvailableVideoCodecCount();
	virtual VideoCodecType getAvailableVideoCodec(size_t id);
    virtual size_t getAvailableVideoContainerCount();
    virtual VideoContainerType getAvailableVideoContainer(size_t id);
};

RPCAM_EXPORTS std::vector<CaptureBackend> getAvailableCaptureBackends();
RPCAM_EXPORTS std::shared_ptr<CameraEnumerator> getCameraEnumerator(CaptureBackend backend = CaptureBackend::Any);
RPCAM_EXPORTS std::shared_ptr<CameraInterface> getCameraInterface(CaptureBackend backend = CaptureBackend::Any);



inline std::shared_ptr<ImageData> CameraInterface::getNewFrame(bool skipOldFrames)
{
	return std::shared_ptr<ImageData>(getNewFramePtr(skipOldFrames), deleteImageDataRawPtr);
}

inline std::vector<ImageFormat> CameraInterface::getListAvailableFormat()
{
	std::vector<ImageFormat> list(getAvailableFormatCount());
	for(size_t i = 0; i < list.size(); i++)
		list[i] = getAvailableFormat(i);
	return list;
}

inline std::vector<VideoCodecType> CameraInterface::getListAvailableVideoCodec()
{
	std::vector<VideoCodecType> list(getAvailableVideoCodecCount());
	for(size_t i = 0; i < list.size(); i++)
		list[i] = getAvailableVideoCodec(i);
	return list;
}

inline std::vector<VideoContainerType> CameraInterface::getListAvailableVideoContainer()
{
	std::vector<VideoContainerType> list(getAvailableVideoContainerCount());
	for(size_t i = 0; i < list.size(); i++)
		list[i] = getAvailableVideoContainer(i);
	return list;
}

}//RPCameraInterface
