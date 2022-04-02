#ifndef CAMERAINTERFACEOPENCV_H
#define CAMERAINTERFACEOPENCV_H

#include "CameraInterfaceBase.h"
#include <opencv2/opencv.hpp>

namespace RPCameraInterface
{

class CameraInterfaceOpenCV : public CameraInterfaceBase
{
public:
    CameraInterfaceOpenCV();
    ~CameraInterfaceOpenCV();
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
    bool testResolution(int width, int height, bool use_mjpg);
    void testAvailableFormats();
    std::vector<ImageFormat> listFormats;
    ImageFormat imageFormat;
    cv::VideoCapture *cap;
    std::string errorMsg;
    int default_fourcc, mjpg_fourcc;
    cv::Mat frame;
};

}

#endif // CAMERAINTERFACELIBWEBCAM_H
