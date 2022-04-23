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
#include <libavformat/avformat.h>
}

#include "RPCameraInterfaceDefs.h"
#include "ImageData.h"

namespace RPCameraInterface
{

//based on https://ffmpeg.org/doxygen/trunk/encode_video_8c-example.html
class RPCAM_EXPORTS VideoEncoder
{
public:
	virtual ~VideoEncoder();
    virtual bool open(const char *filename, int height, int width, int fps = 30, const char *encoderName = "", int bitrate = 2000000, const char *preset = "fast") = 0;
    virtual bool write(const std::shared_ptr<ImageData>& img) = 0;
    virtual void release() = 0;
    virtual void setUseFrameTimestamp(bool useFrameTimestamp) = 0;
};

extern "C" {
	RPCAM_EXPORTS VideoEncoder *createVideoEncoderRawPtr();
	RPCAM_EXPORTS void deleteVideoEncoderRawPtr(VideoEncoder *videoEncoder);
}

inline std::shared_ptr<VideoEncoder> createVideoEncoder()
{
	return std::shared_ptr<VideoEncoder>(createVideoEncoderRawPtr(), deleteVideoEncoderRawPtr);
}

}
