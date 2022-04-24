#include <RPCameraInterface/VideoEncoder.h>
#include <RPCameraInterface/ImageFormatConverter.h>
#include <stdio.h>


#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

namespace RPCameraInterface
{

// a wrapper around a single output AVStream
typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;
 
    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;
 
    AVFrame *frame;
    AVFrame *tmp_frame;
 
    AVPacket *tmp_pkt;
 
    float t, tincr, tincr2;
 
    struct SwsContext *sws_ctx;
    //struct SwrContext *swr_ctx;
} OutputStream;

static bool write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c,
                       AVStream *st, AVFrame *frame, AVPacket *pkt)
{
    int ret;
 
    // send the frame to the encoder
    ret = avcodec_send_frame(c, frame);
    if (ret < 0) {
        printf("Error sending a frame to the encoder\n");
        return false;
    }
 
    while (ret >= 0) {
        ret = avcodec_receive_packet(c, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0) {
            printf("Error encoding a frame\n");
            return false;
        }
 
        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(pkt, c->time_base, st->time_base);
        pkt->stream_index = st->index;
 
        /* Write the compressed frame to the media file. */
        //log_packet(fmt_ctx, pkt);
        ret = av_interleaved_write_frame(fmt_ctx, pkt);
        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets pkt), so that no unreferencing is necessary.
         * This would be different if one used av_write_frame(). */
        if (ret < 0) {
            printf("Error while writing output packet\n");
            return false;
        }
    }
 
    return true;//ret == AVERROR_EOF ? 1 : 0;
}

/* Add an output stream. */
static bool add_stream(OutputStream *ost, AVFormatContext *oc, const AVCodec **codec, enum AVCodecID codec_id, int height, int width, int fps, int bitrate, bool timebase_in_ms)
{
    AVCodecContext *c;
    int i;
 
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        printf("Could not find encoder\n");
        return false;
    }
 
    ost->tmp_pkt = av_packet_alloc();
    if (!ost->tmp_pkt) {
        printf("Could not allocate AVPacket\n");
        return false;
    }
 
    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        printf("Could not allocate stream\n");
        return false;
    }
    ost->st->id = oc->nb_streams-1;
    c = avcodec_alloc_context3(*codec);
    if (!c) {
        printf("Could not alloc an encoding context\n");
        return false;
    }
    ost->enc = c;
 
    switch ((*codec)->type) {
    /*case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt  = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }
        av_channel_layout_copy(&c->ch_layout, &(AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO);
        ost->st->time_base.num = 1;
        ost->st->time_base.den = c->sample_rate;
        break;*/
 
    case AVMEDIA_TYPE_VIDEO:
        c->codec_id = codec_id;
 
        c->bit_rate = bitrate;
        /* Resolution must be a multiple of two. */
        c->width    = width;
        c->height   = height;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
        ost->st->time_base.num = 1;
        if(timebase_in_ms)
            ost->st->time_base.den  = 1000;//fps;
        else ost->st->time_base.den  = fps;
        c->time_base       = ost->st->time_base;
        c->framerate.num = fps;
        c->framerate.den = 1;
 
        c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
        c->pix_fmt       = AV_PIX_FMT_YUV420P;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            /* just for testing, we also add B-frames */
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
        }
        break;
 
    default:
        break;
    }
 
    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    return true;
}

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;
 
    picture = av_frame_alloc();
    if (!picture)
        return NULL;
 
    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;
 
    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 0);
    if (ret < 0) {
        printf("Could not allocate frame data.\n");
        return NULL;
    }
 
    return picture;
}

static bool open_video(AVFormatContext *oc, const AVCodec *codec,
                       OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;
    AVCodecContext *c = ost->enc;
    AVDictionary *opt = NULL;
 
    av_dict_copy(&opt, opt_arg, 0);
 
    /* open the codec */
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        printf("Could not open video codec\n");
        return false;
    }
 
    /* allocate and init a re-usable frame */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!ost->frame) {
        printf("Could not allocate video frame\n");
        return false;
    }
 
    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    ost->tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!ost->tmp_frame) {
            printf("Could not allocate temporary picture\n");
            return false;
        }
    }
 
    if(ost->st->codecpar == NULL)
        printf("ost->st->codecpar is NULL\n");
    
    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        printf("Could not copy the stream parameters\n");
        return false;
    }
    printf("end\n");
    return true;
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    av_packet_free(&ost->tmp_pkt);
    sws_freeContext(ost->sws_ctx);
    //swr_free(&ost->swr_ctx);
}

class RPCAM_EXPORTS VideoEncoderImpl : public VideoEncoder
{
public:
	VideoEncoderImpl();
	virtual ~VideoEncoderImpl();
    virtual bool open(const char *filename, int height, int width, int fps = 30, const char *encoderName = "", int bitrate = 2000000, const char *preset = "fast");
    virtual bool write(const std::shared_ptr<ImageData>& img);
    virtual void release();
    virtual void setUseFrameTimestamp(bool useFrameTimestamp);
private:
    const AVOutputFormat *fmt;
    OutputStream video_st;
    AVFormatContext *oc;
    const AVCodec *audio_codec, *video_codec;
    
    FILE *file;
    int nbEncodedFrames;
    bool useFrameTimestamp;
    int fps;
    uint64_t firstFrameTimestamp;
};

VideoEncoder::~VideoEncoder()
{
}

VideoEncoderImpl::VideoEncoderImpl()
{
    useFrameTimestamp = false;
    fps = 30;
    memset(&video_st, 0, sizeof(video_st));
}

VideoEncoderImpl::~VideoEncoderImpl()
{
	if(video_st.enc != NULL)
		release();
}

void VideoEncoderImpl::setUseFrameTimestamp(bool useFrameTimestamp)
{
    this->useFrameTimestamp = useFrameTimestamp;
}

bool VideoEncoderImpl::open(const char *filename, int height, int width, int fps, const char *encoderName, int bitrate, const char *preset)
{
    nbEncodedFrames = 0;
    this->fps = fps;

    AVDictionary *opt = NULL;

    int ret;

    avformat_alloc_output_context2(&oc, NULL, NULL, filename);
    if (!oc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
    }
    if (!oc) {
        return false;
    }
    fmt = oc->oformat;

    if (fmt->video_codec != AV_CODEC_ID_NONE) {
        printf("add_stream\n");
        add_stream(&video_st, oc, &video_codec, fmt->video_codec, height, width, fps, bitrate, useFrameTimestamp);
    }
    /*if (fmt->audio_codec != AV_CODEC_ID_NONE) {
        add_stream(&audio_st, oc, &audio_codec, fmt->audio_codec);
        have_audio = 1;
        encode_audio = 1;
    }*/

    open_video(oc, video_codec, &video_st, opt);
 
    //if (have_audio)
    //    open_audio(oc, audio_codec, &audio_st, opt);


    if (!(fmt->flags & AVFMT_NOFILE)) {
        printf("avio_open\n");
        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("Could not open '%s'\n", filename);
            return false;
        }
    }

    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        printf("Error occurred when opening output file\n");
        return false;
    }

    return true;
}

bool VideoEncoderImpl::write(const std::shared_ptr<ImageData>& img)
{
    int ret = av_frame_make_writable(video_st.frame);
    if (ret < 0)
        return false;

    const int in_linesize[1] = { 3 * video_st.enc->width };
    if (!video_st.sws_ctx) {
        video_st.sws_ctx = sws_getContext(video_st.enc->width, video_st.enc->height,
                                        ImageTypeToAVPixelFormat(img->getImageFormat().type),
                                        video_st.enc->width, video_st.enc->height,
                                        video_st.enc->pix_fmt,
                                        SWS_BICUBIC, NULL, NULL, NULL);
        if (!video_st.sws_ctx) {
            printf("Could not initialize the conversion context\n");
            return false;
        }
    }
    /*video_st.sws_ctx = sws_getCachedContext(video_st.sws_ctx,
                video_st.enc->width, video_st.enc->height, ImageTypeToAVPixelFormat(img->getImageFormat().type),
                video_st.enc->width, video_st.enc->height, AV_PIX_FMT_YUV420P,
                0, 0, 0, 0);*/
    unsigned char *data = img->getDataPtr();
    sws_scale(video_st.sws_ctx, (const uint8_t * const *)&data, in_linesize, 0,
            video_st.enc->height, video_st.frame->data, video_st.frame->linesize);

    if(useFrameTimestamp) {
        if(nbEncodedFrames == 0)
            firstFrameTimestamp = img->getTimestamp();
        video_st.frame->pts = static_cast<int64_t>(static_cast<double>(img->getTimestamp() - firstFrameTimestamp) + 0.5); //* fps / 1000
    } else {
        video_st.frame->pts = nbEncodedFrames;
    }
    //frame->pkt_pts = frame->pts;
    //frame->pkt_dts = nbEncodedFrames;

    if(!write_frame(oc, video_st.enc, video_st.st, video_st.frame, video_st.tmp_pkt))
    {
        printf("!encode\n");
        return false;
    }

    nbEncodedFrames++;
    return true;
}

void VideoEncoderImpl::release()
{

    /* flush the encoder */
    write_frame(oc, video_st.enc, video_st.st, NULL, video_st.tmp_pkt);
    //encode(codecContext, NULL, pkt, file);

    av_write_trailer(oc);

    //if (have_video)
        close_stream(oc, &video_st);
    //if (have_audio)
    //    close_stream(oc, &audio_st);

    if (!(fmt->flags & AVFMT_NOFILE))
        avio_closep(&oc->pb);
    
    avformat_free_context(oc);

    video_st.enc = NULL;
}

RPCAM_EXPORTS VideoEncoder *createVideoEncoderRawPtr()
{
	return new VideoEncoderImpl();
}

RPCAM_EXPORTS void deleteVideoEncoderRawPtr(VideoEncoder *videoEncoder)
{
	delete videoEncoder;
}

}
