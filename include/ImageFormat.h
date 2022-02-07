#ifndef IMAGEFORMAT_H
#define IMAGEFORMAT_H

#include <string>

namespace RPCameraInterface
{

enum class ImageType
{
    UNKNOWN,
    RGB,
    BGR,
    YUYV422,
    MJPG,
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

std::string toString(ImageType type);

class ImageFormat
{
public:
    int width, height;
    int fps;
    ImageType type;

    ImageFormat();

    std::string toString();
};

}

#endif // IMAGEFORMAT_H
