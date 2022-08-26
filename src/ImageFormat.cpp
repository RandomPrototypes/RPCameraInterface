#include <RPCameraInterface/ImageFormat.h>
#include <sstream>
#include <cstring>

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

PortableString ImageFormat::toString()
{
    std::ostringstream str;
    str << RPCameraInterface::toString(type) << ", " << width << "x" << height << ", " << fps << "fps";
    return toPortableString(str.str());
}

ImageType toImageType(const char *guid)
{

    if(!strcmp(guid, "AYUV")) {
        return ImageType::AYUV;
    } else if(!strcmp(guid, "RGB") || !strcmp(guid, "RGB24")) {
        return ImageType::RGB24;
    } else if(!strcmp(guid, "RGBA") || !strcmp(guid, "RGB32") || !strcmp(guid, "RGBA32")) {
        return ImageType::RGBA32;
    } else if(!strcmp(guid, "BGR") || !strcmp(guid, "BGR24")) {
        return ImageType::BGR24;
    } else if(!strcmp(guid, "BGRA") || !strcmp(guid, "BGR32") || !strcmp(guid, "BGRA32")) {
        return ImageType::BGRA32;
    }else if(!strcmp(guid, "RGB555")) {
        return ImageType::RGB555;
    } else if(!strcmp(guid, "RGB565")) {
        return ImageType::RGB565;
    } else if(!strcmp(guid, "IYUV") || !strcmp(guid, "I420") || !strcmp(guid, "YV12") || !strcmp(guid, "YUV420P")) {
        return ImageType::YUV420P;
    } else if(!strcmp(guid, "UYVY")) {
        return ImageType::UYVY;
    } else if(!strcmp(guid, "Y211")) {
        return ImageType::Y211;
    } else if(!strcmp(guid, "Y411") || !strcmp(guid, "Y41P")) {
        return ImageType::Y41P;
    } else if(!strcmp(guid, "YUY2") || !strcmp(guid, "YUYV")) {
        return ImageType::YUYV422;
    } else if(!strcmp(guid, "YVU9")) {
        return ImageType::YVU9;
    } else if(!strcmp(guid, "YVYU")) {
        return ImageType::YVYU;
    } else if(!strcmp(guid, "Y8") || !strcmp(guid, "Y800") || !strcmp(guid, "GREY") || !strcmp(guid, "GRAY8")) {
        return ImageType::GRAY8;
    } else if(!strcmp(guid, "BY8")) {
        return ImageType::BY8;
    } else if(!strcmp(guid, "Y16")) {
        return ImageType::Y16;
    } else if(!strcmp(guid, "NV12")) {
        return ImageType::NV12;
    } else if(!strcmp(guid, "NV21")) {
        return ImageType::NV21;
    } else if(!strcmp(guid, "JPG") || !strcmp(guid, "MJPG")) {
        return ImageType::JPG;
    } else {
        return ImageType::UNKNOWN;
    }
}

}
