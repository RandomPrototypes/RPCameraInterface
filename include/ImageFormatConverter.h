#ifndef IMAGEFORMATCONVERTER_HPP
#define IMAGEFORMATCONVERTER_HPP

#include <memory>
#include "ImageFormat.h"
#include "ImageData.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/error.h>
}

namespace RPCameraInterface
{

class ImageFormatConverter
{
public:
    ImageFormatConverter(ImageFormat srcFormat, ImageFormat dstFormat);
    ~ImageFormatConverter();

    void init(ImageFormat srcFormat, ImageFormat dstFormat);

    bool convertImage(const std::shared_ptr<ImageData>& srcImg, std::shared_ptr<ImageData> dstImg);


    SwsContext *swsContext = nullptr;
    AVCodec *codec = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVPacket *pkt  = nullptr;
    AVFrame *frame = nullptr;


    ImageFormat srcFormat;
    ImageFormat dstFormat;
    int src_linesize[4];
    int dst_linesize[4];
    AVPixelFormat srcPixelFormat, dstPixelFormat;
};

AVPixelFormat ImageTypeToAVPixelFormat(ImageType type);

}

#endif // IMAGEFORMATCONVERTER_HPP
