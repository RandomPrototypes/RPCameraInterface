#pragma once

#include "CameraInterfaceBase.h"
#include <sstream>
#include <libwebcam/webcam.h>

namespace RPCameraInterface
{

class CameraEnumeratorLibWebcam : public CameraEnumeratorBase
{
public:
    CameraEnumeratorLibWebcam();
    virtual ~CameraEnumeratorLibWebcam();

    virtual bool detectCameras();
};

class CameraInterfaceLibWebcam : public CameraInterfaceBase
{
public:
    CameraInterfaceLibWebcam();
    ~CameraInterfaceLibWebcam();
    virtual bool open(const char *params);
    virtual bool close();
    virtual void selectFormat(int formatId);
    virtual void selectFormat(ImageFormat format);
    virtual const char *getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();
private:
    ImageType toImageType(const webcam::format &format) const;

    int deviceId;
    std::vector<ImageFormat> listFormats;
    std::vector<webcam::format*> listLibWebcamFormats;
    ImageFormat imageFormat;
    webcam::device *device;
    std::string errorMsg;
};

}
