#pragma once

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
}

#include "RPCameraInterfaceDefs.h"
#include "ImageData.h"

namespace RPCameraInterface
{

//based on https://ffmpeg.org/doxygen/trunk/encode_video_8c-example.html
class RPCAM_EXPORTS H264Encoder
{
public:
	virtual ~H264Encoder();
    virtual bool open(const char *filename, int height, int width, int fps = 30, int bitrate = 2000000, const char *preset = "fast") = 0;
    virtual bool write(const std::shared_ptr<ImageData>& img) = 0;
    virtual void release() = 0;
};

extern "C" {
	RPCAM_EXPORTS H264Encoder *createH264EncoderRawPtr();
	RPCAM_EXPORTS void deleteH264EncoderRawPtr(H264Encoder *h264Encoder);
}

inline std::shared_ptr<H264Encoder> createH264Encoder()
{
	return std::shared_ptr<H264Encoder>(createH264EncoderRawPtr(), deleteH264EncoderRawPtr);
}

}
