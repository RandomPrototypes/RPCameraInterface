#ifndef CAMERAINTERFACEDSHOW_H
#define CAMERAINTERFACEDSHOW_H

#include "CameraInterfaceBase.h"
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

class CameraEnumeratorDShow : public CameraEnumeratorBase
{
public:
    CameraEnumeratorDShow();
    virtual ~CameraEnumeratorDShow();

    virtual bool detectCameras();
};

class CameraInterfaceDShow : public CameraInterfaceBase
{
public:
    CameraInterfaceDShow();
    ~CameraInterfaceDShow();
    virtual bool open(const char *params);
    virtual bool close();
    virtual void selectFormat(int formatId);
    virtual void selectFormat(ImageFormat format);
    virtual const char *getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);

private:
    std::vector<ImageFormat> listFormats;
    std::vector<int> listFormatsFourcc;
    ImageFormat imageFormat;
    std::string errorMsg;
    int cameraId;
};

}

#endif // CAMERAINTERFACEDSHOW_H
