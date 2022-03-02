#ifndef CAMERAINTERFACEDSHOW_H
#define CAMERAINTERFACEDSHOW_H

#include "CameraInterface.h"
#include <sstream>

namespace RPCameraInterface
{

class videoInput;

class DShowVideoInput
{
private:
    DShowVideoInput();
    ~DShowVideoInput();
    videoInput *g_VI;
    static DShowVideoInput *instance;
public:
    static videoInput& getVideoInput();
};

class CameraEnumeratorDShow : public CameraEnumerator
{
public:
    CameraEnumeratorDShow();
    virtual ~CameraEnumeratorDShow();

    virtual bool detectCameras();
};

class CameraInterfaceDShow : public CameraInterface
{
public:
    CameraInterfaceDShow();
    ~CameraInterfaceDShow();
    virtual bool open(std::string params);
    virtual bool close();
    virtual std::vector<ImageFormat> getAvailableFormats();
    virtual void selectFormat(int formatId);
    virtual void selectFormat(ImageFormat format);
    virtual std::shared_ptr<ImageData> getNewFrame(bool skipOldFrames);
    virtual std::string getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();
private:
    std::vector<ImageFormat> listFormats;
    std::vector<int> listFormatsFourcc;
    ImageFormat imageFormat;
    std::string errorMsg;
    int cameraId;
};

class CameraInterfaceFactoryDShow : public CameraInterfaceFactory
{
public:
    CameraInterfaceFactoryDShow();
    virtual ~CameraInterfaceFactoryDShow();
    virtual CameraInterface *createInterface();
};

}

#endif // CAMERAINTERFACEDSHOW_H
