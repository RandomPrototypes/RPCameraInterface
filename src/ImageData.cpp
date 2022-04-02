#include <RPCameraInterface/ImageData.h>

namespace RPCameraInterface
{

class RPCAM_EXPORTS ImageDataImpl : public ImageData
{
public:
    ImageDataImpl(ImageFormat imageFormat = ImageFormat(), unsigned char *data = NULL, int dataSize = 0, uint64_t timestamp = 0);
    virtual ~ImageDataImpl();
    virtual void allocData(int size);
    virtual void freeData();
    
    virtual const ImageFormat& getImageFormat() const;
    virtual ImageFormat& getImageFormat();
    virtual void setImageFormat(ImageFormat format);
    virtual uint64_t getTimestamp() const;
    virtual void setTimestamp(uint64_t timestamp);
    virtual unsigned char *getDataPtr() const;
    virtual void setDataPtr(unsigned char *dataPtr);
    virtual int getDataSize() const;
    virtual void setDataSize(int size);
    virtual bool isDataReleasedWhenDestroy() const;
    virtual void setDataReleasedWhenDestroy(bool val);
    
private:
	ImageFormat imageFormat;
    uint64_t timestamp;
    unsigned char *data;
    int dataSize;
    bool releaseDataWhenDestroy;
};

ImageData::~ImageData()
{
}

ImageDataImpl::ImageDataImpl(ImageFormat imageFormat, unsigned char *data, int dataSize, uint64_t timestamp)
    :imageFormat(imageFormat), data(data), dataSize(dataSize), timestamp(timestamp)
{
    releaseDataWhenDestroy = true;
}

ImageDataImpl::~ImageDataImpl()
{
    if(releaseDataWhenDestroy && data != NULL)
        delete [] data;
}

void ImageDataImpl::allocData(int size)
{
    data = new unsigned char[size];
    dataSize = size;
}

void ImageDataImpl::freeData()
{
    delete [] data;
    data = NULL;
    dataSize = 0;
}


const ImageFormat& ImageDataImpl::getImageFormat() const
{
	return imageFormat;
}
ImageFormat& ImageDataImpl::getImageFormat()
{
	return imageFormat;
}
void ImageDataImpl::setImageFormat(ImageFormat format)
{
	imageFormat = format;
}
uint64_t ImageDataImpl::getTimestamp() const
{
	return timestamp;
}
void ImageDataImpl::setTimestamp(uint64_t timestamp)
{
	this->timestamp = timestamp;
}
unsigned char *ImageDataImpl::getDataPtr() const
{
	return data;
}
void ImageDataImpl::setDataPtr(unsigned char *dataPtr)
{
	data = dataPtr;
}
int ImageDataImpl::getDataSize() const
{
	return dataSize;
}
void ImageDataImpl::setDataSize(int size)
{
	dataSize = size;
}
bool ImageDataImpl::isDataReleasedWhenDestroy() const
{
	return releaseDataWhenDestroy;
}
void ImageDataImpl::setDataReleasedWhenDestroy(bool val)
{
	releaseDataWhenDestroy = val;
}

RPCAM_EXPORTS ImageData *createImageDataRawPtr(ImageFormat imageFormat, unsigned char *data, int dataSize, uint64_t timestamp)
{
	return new ImageDataImpl(imageFormat, data, dataSize, timestamp);
}

RPCAM_EXPORTS void deleteImageDataRawPtr(ImageData *img)
{
	delete img;
}

}
