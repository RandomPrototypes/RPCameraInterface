#pragma once

#include <memory>
#include "ImageFormat.h"
#include "ImageData.h"
#include "RPCameraInterfaceDefs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/error.h>
}

namespace RPCameraInterface
{

class RPCAM_EXPORTS ImageFormatConverter
{
public:
    ImageFormatConverter(ImageFormat srcFormat, ImageFormat dstFormat);
    ~ImageFormatConverter();

    void init(ImageFormat srcFormat, ImageFormat dstFormat);

    bool convertImage(const std::shared_ptr<ImageData>& srcImg, std::shared_ptr<ImageData> dstImg);


    SwsContext *swsContext = nullptr;
    const AVCodec *codec = nullptr;
    AVCodecContext *codecContext = nullptr;
    AVPacket *pkt  = nullptr;
    AVFrame *frame = nullptr;


    ImageFormat srcFormat;
    ImageFormat dstFormat;
    int src_linesize[4];
    int dst_linesize[4];
    AVPixelFormat srcPixelFormat, dstPixelFormat;
};

AVPixelFormat RPCAM_EXPORTS ImageTypeToAVPixelFormat(ImageType type);

}
