#pragma once

#include "CameraInterfaceBase.h"
#include "ScreenCapture.h"
#include <mutex>


namespace RPCameraInterface
{

class CameraEnumeratorScreenCaptureLite : public CameraEnumeratorBase
{
public:
    CameraEnumeratorScreenCaptureLite();
    virtual ~CameraEnumeratorScreenCaptureLite();

    virtual bool detectCameras();
};

class CameraInterfaceScreenCaptureLite : public CameraInterfaceBase
{
public:
    CameraInterfaceScreenCaptureLite();
    ~CameraInterfaceScreenCaptureLite();
    virtual bool open(const char *params);
    virtual bool close();
    virtual void selectFormat(int formatId);
    virtual void selectFormat(ImageFormat format);
    virtual const char *getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();
    
    virtual bool hasROICapability();
    virtual bool setROI(int x, int y, int width, int height);
    virtual void getROI(int *x, int *y, int *width, int *height) const;

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);

private:
    ImageFormat imageFormat;
    std::string winName;
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> frameGrabber;
    ImageData *currentImage;
    std::vector<ImageData*> recentImages;
    std::vector<ImageData*> memoryBuffer;
    std::mutex mutex;
    int maxBufferSize;
    std::string errorMsg;
    int ROI_x, ROI_y, ROI_width, ROI_height;
};

}
