#ifndef CAMERAINTERFACEV4L2_H
#define CAMERAINTERFACEV4L2_H

#include "CameraInterfaceBase.h"
#include <sstream>

namespace RPCameraInterface
{

enum io_method {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct V4L2_buffer {
    void   *start;
    size_t  length;
};

class CameraEnumeratorV4L2 : public CameraEnumeratorBase
{
public:
    CameraEnumeratorV4L2();
    virtual ~CameraEnumeratorV4L2();

    virtual bool detectCameras();
};

class CameraInterfaceV4L2 : public CameraInterfaceBase
{
public:
    CameraInterfaceV4L2();
    ~CameraInterfaceV4L2();
    virtual bool open(const char *params);
    virtual bool close();
    virtual void selectFormat(int formatId);
    virtual void selectPreviewFormat(int formatId);
    virtual void selectFormat(ImageFormat format);
    virtual void selectPreviewFormat(ImageFormat format);
    virtual const char *getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);

private:
    bool initDevice();
    bool uninitDevice();
    bool init_read(unsigned int buffer_size);
    bool init_mmap();
    bool init_userp(unsigned int buffer_size);
    int read_frame();
    void requestImageFormatList();

    void process_image(const void *p, int size);

    int fd;
    std::string dev_name;
    std::ostringstream errorMsg;
    std::string errorMsgStr;

    int selectedFormat, selectedPreviewFormat;

    ImageFormat imageFormat;

    struct V4L2_buffer *buffers;

    std::vector<ImageFormat> listImageFormats;

    io_method io = IO_METHOD_MMAP;

    unsigned char *lastFrameData;
    int lastFrameDataLength;
    int lastFrameDataAllocatedSize;

    unsigned int n_buffers;
};

}

#endif // CAMERAINTERFACEV4L2_H
