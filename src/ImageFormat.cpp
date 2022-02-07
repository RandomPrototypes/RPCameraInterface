#include "ImageFormat.h"
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

std::string ImageFormat::toString()
{
    std::ostringstream str;
    str << RPCameraInterface::toString(type) << ", " << width << "x" << height << ", " << fps << "fps";
    return str.str();
}

}
