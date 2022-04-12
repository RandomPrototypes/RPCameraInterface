#pragma once

#include "RPCameraInterfaceDefs.h"
#include "ImageFormat.h"
#include <memory>

namespace RPCameraInterface
{

class RPCAM_EXPORTS ImageData
{
public:
    virtual ~ImageData();
    virtual void allocData(int size) = 0;
    virtual void freeData() = 0;
    
    virtual const ImageFormat& getImageFormat() const = 0;
    virtual ImageFormat& getImageFormat() = 0;
    virtual void setImageFormat(ImageFormat format) = 0;
    virtual uint64_t getTimestamp() const = 0;
    virtual void setTimestamp(uint64_t timestamp) = 0;
    virtual unsigned char *getDataPtr() const = 0;
    virtual void setDataPtr(unsigned char *dataPtr) = 0;
    virtual int getDataSize() const = 0;
    virtual void setDataSize(int size) = 0;
    virtual bool isDataReleasedWhenDestroy() const = 0;
    virtual void setDataReleasedWhenDestroy(bool val) = 0;
};

extern "C" {
	RPCAM_EXPORTS ImageData *createImageDataRawPtr(ImageFormat imageFormat = ImageFormat(), unsigned char *data = NULL, int dataSize = 0, uint64_t timestamp = 0);
	RPCAM_EXPORTS void deleteImageDataRawPtr(ImageData *img);
}

inline std::shared_ptr<ImageData> createImageData(ImageFormat imageFormat = ImageFormat(), unsigned char *data = NULL, int dataSize = 0, uint64_t timestamp = 0)
{
	return std::shared_ptr<ImageData>(createImageDataRawPtr(imageFormat, data, dataSize, timestamp), deleteImageDataRawPtr);
}

}
