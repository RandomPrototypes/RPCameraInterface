#pragma once

#include "CameraInterfaceBase.h"
#include "depthai/depthai.hpp"
#include "depthai/xlink/XLinkConnection.hpp"

namespace RPCameraInterface
{

class CameraEnumeratorDepthAI : public CameraEnumeratorBase
{
public:
    CameraEnumeratorDepthAI();
    virtual ~CameraEnumeratorDepthAI();

    virtual bool detectCameras();
};

class CameraInterfaceDepthAI : public CameraInterfaceBase
{
public:
    CameraInterfaceDepthAI();
    ~CameraInterfaceDepthAI();
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

    void detectAvailableFormats();

private:
    std::vector<ImageFormat> listFormats;
    ImageFormat imageFormat;
    std::string errorMsg;

    dai::Pipeline pipeline;
    dai::Device *device;
    dai::DeviceInfo deviceInfo;
    std::shared_ptr<dai::node::ColorCamera> camRgb;
    std::shared_ptr<dai::node::XLinkOut> xoutVideo;
    std::shared_ptr<dai::DataOutputQueue> video;
    std::shared_ptr<dai::node::VideoEncoder> videnc;
    std::shared_ptr<dai::ImgFrame> videoIn;
    cv::Mat img;
};

}
