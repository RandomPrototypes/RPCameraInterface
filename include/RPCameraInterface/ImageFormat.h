#ifndef IMAGEFORMAT_H
#define IMAGEFORMAT_H

#include <string>
#include "RPCameraInterfaceDefs.h"

namespace RPCameraInterface
{

enum class ImageType
{
    UNKNOWN,
    GRAY8,
    RGB24,
    BGR24,
    BGRA32,
    RGB555,
    RGB565,
    AYUV,
    YUV420P,
    UYVY,
    Y211,
    Y41P,
    YUYV422,
    YVU9,
    YVYU,
    BY8,
    Y16,
    JPG,
    NB_FORMAT_TYPE
};

enum class VideoCodecType
{
    UNKNOWN,
    H264,
};

enum class VideoContainerType
{
    UNKNOWN,
    NONE,
    MP4
};

RPCAM_EXPORTS std::string toString(ImageType type);

class RPCAM_EXPORTS ImageFormat
{
public:
    int width, height;
    int fps;
    ImageType type;

    ImageFormat();
    ImageFormat(ImageType type, int width, int height, int fps = 0);

    std::string toString();
};

}

#endif // IMAGEFORMAT_H
