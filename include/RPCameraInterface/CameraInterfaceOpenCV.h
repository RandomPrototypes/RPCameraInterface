#ifndef CAMERAINTERFACEOPENCV_H
#define CAMERAINTERFACEOPENCV_H

#include "CameraInterface.h"
#include <opencv2/opencv.hpp>

namespace RPCameraInterface
{

class CameraInterfaceOpenCV : public CameraInterface
{
public:
    CameraInterfaceOpenCV();
    ~CameraInterfaceOpenCV();
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
    bool testResolution(int width, int height, bool use_mjpg);
    std::vector<ImageFormat> listFormats;
    ImageFormat imageFormat;
    cv::VideoCapture *cap;
    std::string errorMsg;
    int default_fourcc, mjpg_fourcc;
    cv::Mat frame;
};

}

#endif // CAMERAINTERFACELIBWEBCAM_H
