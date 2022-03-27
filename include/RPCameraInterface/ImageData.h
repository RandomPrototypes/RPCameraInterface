#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include "RPCameraInterfaceDefs.h"

#include "ImageFormat.h"

namespace RPCameraInterface
{

class RPCAM_EXPORTS ImageData
{
public:
    ImageFormat imageFormat;
    uint64_t timestamp;
    unsigned char *data;
    int dataSize;
    bool releaseDataWhenDestroy;

    ImageData(ImageFormat imageFormat = ImageFormat(), unsigned char *data = NULL, int dataSize = 0, uint64_t timestamp = 0);
    ~ImageData();
    void allocData(int size);
    void freeData();
};

}

#endif // IMAGEDATA_H
