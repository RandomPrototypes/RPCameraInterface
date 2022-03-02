#include <RPCameraInterface/ImageData.h>

namespace RPCameraInterface
{

ImageData::ImageData(ImageFormat imageFormat, unsigned char *data, int dataSize, uint64_t timestamp)
    :imageFormat(imageFormat), data(data), dataSize(dataSize), timestamp(timestamp)
{
    releaseDataWhenDestroy = true;
}

ImageData::~ImageData()
{
    if(releaseDataWhenDestroy && data != NULL)
        delete [] data;
}

void ImageData::allocData(int size)
{
    data = new unsigned char[size];
    dataSize = size;
}

void ImageData::freeData()
{
    delete [] data;
    data = NULL;
    dataSize = 0;
}

}
