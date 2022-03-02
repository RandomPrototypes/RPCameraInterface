#include <RPCameraInterface/ImageFormat.h>
#include <sstream>


namespace RPCameraInterface
{

ImageFormat::ImageFormat()
{
    type = ImageType::UNKNOWN;
    width = 0;
    height = 0;
    fps = 0;
}

ImageFormat::ImageFormat(ImageType type, int width, int height, int fps)
    :type(type), width(width), height(height), fps(fps)
{

}

std::string ImageFormat::toString()
{
    std::ostringstream str;
    str << RPCameraInterface::toString(type) << ", " << width << "x" << height << ", " << fps << "fps";
    return str.str();
}

}
