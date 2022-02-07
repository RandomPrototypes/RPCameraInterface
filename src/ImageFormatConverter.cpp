#include "ImageFormatConverter.h"

namespace RPCameraInterface
{

AVPixelFormat ImageTypeToAVPixelFormat(ImageType type)
{
    if(type == ImageType::RGB)
        return AV_PIX_FMT_RGB24;
    else if(type == ImageType::BGR)
        return AV_PIX_FMT_BGR24;
    else if(type == ImageType::YUYV422)
        return AV_PIX_FMT_YUYV422;
    else return AV_PIX_FMT_NONE;
}


ImageFormatConverter::ImageFormatConverter(ImageFormat srcFormat, ImageFormat dstFormat)
{
    init(srcFormat, dstFormat);
}

ImageFormatConverter::~ImageFormatConverter()
{
    if(swsContext != nullptr)
        sws_freeContext(swsContext);
}

void ImageFormatConverter::init(ImageFormat srcFormat, ImageFormat dstFormat)
{
    this->srcFormat = srcFormat;
    this->dstFormat = dstFormat;

    srcPixelFormat = ImageTypeToAVPixelFormat(srcFormat.type);
    dstPixelFormat = ImageTypeToAVPixelFormat(dstFormat.type);
    //https://stackoverflow.com/questions/23443322/decoding-mjpeg-with-libavcodec
    if(srcFormat.type == ImageType::MJPG)
    {
        srcPixelFormat = AV_PIX_FMT_BGR24;//AV_PIX_FMT_YUVJ422P;
        codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
        codecContext = avcodec_alloc_context3(codec);
        avcodec_get_context_defaults3(codecContext, codec);
        avcodec_open2(codecContext, codec, nullptr);

        pkt = av_packet_alloc();
        if (!pkt) {
            //qDebug() << ("Could not allocate packet\n");
            return ;
        }

        frame = av_frame_alloc();
        if (!frame) {
            //qDebug() << ("Could not allocate video frame\n");
            return ;
        }
        /*frame->format = srcPixelFormat;//codecContext->pix_fmt;
        frame->width  = codecContext->width;
        frame->height = codecContext->height;*/
        //frame->channels = 3;
        /* the image can be allocated by any means and av_image_alloc() is
         * just the most convenient way if av_malloc() is to be used */
        /*int ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            qDebug() << ("Could not allocate the video frame data\n");
            return ;
        }*/
        swsContext = NULL;
    }
    else
    {
        swsContext = sws_getContext(
                            srcFormat.width,
                            srcFormat.height,
                            srcPixelFormat,
                            dstFormat.width,
                            dstFormat.height,
                            dstPixelFormat,
                            SWS_POINT,
                            nullptr, nullptr, nullptr);
    }

    av_image_fill_linesizes(src_linesize, srcPixelFormat, srcFormat.width);
    av_image_fill_linesizes(dst_linesize, dstPixelFormat, dstFormat.width);
    //qDebug() << "src_linesize" << src_linesize[0] << src_linesize[1] << src_linesize[2] << src_linesize[3];
    //qDebug() << "dst_linesize" << dst_linesize[0] << dst_linesize[1] << dst_linesize[2] << dst_linesize[3];
}

bool ImageFormatConverter::convertImage(const std::shared_ptr<ImageData>& srcImg, std::shared_ptr<ImageData> dstImg)
{
    int total_size = dst_linesize[0] + dst_linesize[1] + dst_linesize[2] + dst_linesize[3];
    total_size *= dstFormat.height;

    if(dstImg->dataSize != total_size)
    {
        if(dstImg->data != NULL && dstImg->releaseDataWhenDestroy)
            dstImg->freeData();
        dstImg->allocData(total_size);
        dstImg->releaseDataWhenDestroy = true;
    }

    dstImg->imageFormat = dstFormat;
    dstImg->timestamp = srcImg->timestamp;

    if(srcFormat.type == ImageType::MJPG)
    {
        pkt->size = srcImg->dataSize;
        pkt->data = srcImg->data;

        int got_picture;

        // decode
        //avcodec_decode_video2(codecContext, _outputFrame, &got_picture, &_inputPacket);

        int ret = avcodec_send_packet(codecContext, pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret == AVERROR_EOF) {
            printf("Error sending a frame for decoding\n");
            return false;
        }

        pkt->size = 0;
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret < 0)
            return false;

        //memcpy(dstImg->data, frame->data[0], total_size);

        //qDebug() << "frame->linesize" << frame->linesize[0] << frame->linesize[1] << frame->linesize[2] << frame->linesize[3];

        if(swsContext == NULL) {
            swsContext = sws_getContext(
                                frame->width,
                                frame->height,
                                (AVPixelFormat)frame->format,
                                dstFormat.width,
                                dstFormat.height,
                                dstPixelFormat,
                                SWS_POINT,
                                nullptr, nullptr, nullptr);

        }

        sws_scale(swsContext,
                  (const uint8_t * const *)&(frame->data[0]),
                  frame->linesize,
                  0,
                  srcImg->imageFormat.height,
                  (uint8_t* const*)&(dstImg->data),
                  dst_linesize);
    } else {
        sws_scale(swsContext,
                  (const uint8_t * const *)&(srcImg->data),
                  src_linesize,
                  0,
                  srcImg->imageFormat.height,
                  (uint8_t* const*)&(dstImg->data),
                  dst_linesize);
    }
    return true;
}

}
