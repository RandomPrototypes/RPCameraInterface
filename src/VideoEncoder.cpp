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
#include <vector>

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
    struct SwrContext *swr_ctx;
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
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt  = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = bitrate;
        c->sample_rate = 48000;//44100;
        /*if ((*codec)->supported_samplerates) {
            c->sample_rate = (*codec)->supported_samplerates[0];
            for (i = 0; (*codec)->supported_samplerates[i]; i++) {
                if ((*codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }*/
        c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        if ((*codec)->channel_layouts) {
            c->channel_layout = (*codec)->channel_layouts[0];
            for (i = 0; (*codec)->channel_layouts[i]; i++) {
                if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        ost->st->time_base.num = 1;
        ost->st->time_base.den = c->sample_rate;
        break;
 
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


static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;
    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }
    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;
    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }
    return frame;
}

static bool open_audio(AVFormatContext *oc, const AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    AVCodecContext *c;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;
    c = ost->enc;
    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        fprintf(stderr, "Could not open audio codec\n");
        return false;
    }
    ost->samples_count = 0;
    ost->next_pts = 0;
    /* init signal generator */
    ost->t     = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;
    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;
    ost->frame     = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                                       c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
                                       c->sample_rate, nb_samples);
    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        return false;
    }
    /* create resampler context */
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        return false;
    }
    /* set options */
    av_opt_set_int       (ost->swr_ctx, "in_channel_count",   2,       0);
    av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int       (ost->swr_ctx, "out_channel_count",  c->channels,       0);
    av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     c->sample_fmt,     0);
    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        return false;
    }
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
    virtual bool write_audio(float *buffer, int length, uint64_t timestamp);
    virtual void release();
    virtual void setUseFrameTimestamp(bool useFrameTimestamp);
private:
    void rescaleAndAppendToAudioBuffer(float *buffer, int length, int nbChannel, float speed);
    const AVOutputFormat *fmt;
    OutputStream audio_st, video_st;
    AVFormatContext *oc;
    const AVCodec *audio_codec, *video_codec;
    std::vector<float> audioBuffer;
    
    FILE *file;
    int nbEncodedFrames;
    bool useFrameTimestamp;
    int fps;
    uint64_t firstFrameTimestamp;
    uint64_t firstFrameTimestampAudio;
    float lastAudioVal[2];
};

VideoEncoder::~VideoEncoder()
{
}

VideoEncoderImpl::VideoEncoderImpl()
{
    useFrameTimestamp = false;
    fps = 30;
    firstFrameTimestamp = 0;
    firstFrameTimestampAudio = 0;
    float lastAudioVal[2] = {0.5,0.5};
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
    int have_audio = 0, encode_audio = 0;
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
        add_stream(&video_st, oc, &video_codec, fmt->video_codec, height, width, fps, bitrate, useFrameTimestamp);
    }
    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
        add_stream(&audio_st, oc, &audio_codec, fmt->audio_codec, 0, 0, fps, 64000, false);
        have_audio = 1;
        encode_audio = 1;
    }

    open_video(oc, video_codec, &video_st, opt);
 
    if (have_audio)
        open_audio(oc, audio_codec, &audio_st, opt);


    if (!(fmt->flags & AVFMT_NOFILE)) {
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

static AVFrame *get_audio_frame(OutputStream *ost, float *buffer, int length)
{
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];
 
    for (j = 0; j < frame->nb_samples; j++) {
        if(j*2 >= length)
        {
            for (i = 0; i < 2; i++)
                *q++ = 0;
        } else {
            for (i = 0; i < 2; i++)
                *q++ = (int16_t)(buffer[j*2 + i] * 32767);
        }
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
    }
 
    frame->pts = ost->next_pts;
    ost->next_pts  += frame->nb_samples;
 
    return frame;
}

void VideoEncoderImpl::rescaleAndAppendToAudioBuffer(float *buffer, int length, int nbChannel, float speed)
{
    int nbSampleSrc = length / nbChannel;
    int nbSampleDst = (int)(nbSampleSrc / speed + 0.5);
    for(int i = 0; i < nbSampleDst; i++)
    {
        int i2 = (i * nbSampleSrc + (nbSampleDst/2)) / nbSampleDst;
        for(int j = 0; j < 2; j++) {
            audioBuffer.push_back(buffer[i2*2+j]);
        }
    }
    lastAudioVal[0] = audioBuffer[audioBuffer.size()-2];
    lastAudioVal[1] = audioBuffer[audioBuffer.size()-1];
}

bool VideoEncoderImpl::write_audio(float *buffer, int length, uint64_t timestamp)
{
    if(firstFrameTimestampAudio == 0)
        firstFrameTimestampAudio = timestamp;
    if(length == 0)
        return false;
    AVCodecContext *c;
    AVFrame *frame;
    int ret;
    int dst_nb_samples;
 
    c = audio_st.enc;

    uint64_t timestamp_pts = (timestamp - firstFrameTimestampAudio) * c->sample_rate / 1000;
    uint64_t pts;
    int64_t pts_diff;
    float speed;
    bool firstLoop = true;
    while(true)
    {
        pts = audio_st.next_pts + audioBuffer.size() / 2;

        pts_diff = (int64_t)timestamp_pts - (int64_t)pts;

        int64_t nbRecoverySec = 5;

        //(speed-1)*nbRecoverySec*c->sample_rate = -pts_diff
        speed = 1.0 - static_cast<float>(pts_diff) / (nbRecoverySec*c->sample_rate);
        if((firstLoop && speed >= 0.95) || speed >= 1.0)
            break;
        else {
            audioBuffer.push_back(lastAudioVal[0]);
            audioBuffer.push_back(lastAudioVal[1]);
        }
        firstLoop = false;
    }

    //printf("pts_diff %s, speed %f\n", std::to_string(pts_diff).c_str(), speed);
    rescaleAndAppendToAudioBuffer(buffer, length, 2, speed);

    while(audioBuffer.size() >= audio_st.tmp_frame->nb_samples*2) {
        int current_length = audio_st.tmp_frame->nb_samples*2;//std::min(audio_st.tmp_frame->nb_samples*2, length);
    
        frame = get_audio_frame(&audio_st, &audioBuffer[0], current_length);
        audioBuffer.erase(audioBuffer.begin(), audioBuffer.begin() + current_length);

        pts = frame->pts;
    
        if (frame) {
            /* convert samples from native format to destination codec format, using the resampler */
            /* compute destination number of samples */
            dst_nb_samples = av_rescale_rnd(swr_get_delay(audio_st.swr_ctx, c->sample_rate) + frame->nb_samples,
                                            c->sample_rate, c->sample_rate, AV_ROUND_UP);
            printf("%d == %d\n", dst_nb_samples, frame->nb_samples);
            av_assert0(dst_nb_samples == frame->nb_samples);
    
            /* when we pass a frame to the encoder, it may keep a reference to it
            * internally;
            * make sure we do not overwrite it here
            */
            ret = av_frame_make_writable(audio_st.frame);
            if (ret < 0)
                return false;
    
            /* convert to destination format */
            ret = swr_convert(audio_st.swr_ctx,
                            audio_st.frame->data, dst_nb_samples,
                            (const uint8_t **)frame->data, frame->nb_samples);
            if (ret < 0) {
                printf("Error while converting\n");
                return false;
            }
            frame = audio_st.frame;
    
            /*uint64_t diff_pts = timestamp_pts > pts ? timestamp_pts - pts : pts - timestamp_pts;
            if(diff_pts > c->sample_rate / 5) {
                pts = timestamp_pts;
                audio_st.next_pts = pts + frame->nb_samples;
            }*/
            frame->pts = pts;
            //frame->pts = audio_st.samples_count;//av_rescale_q(audio_st->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
            //frame->pts = (timestamp - firstFrameTimestampAudio) * c->sample_rate / 1000000;

            audio_st.samples_count += dst_nb_samples;
        }
    
        if(!write_frame(oc, c, audio_st.st, frame, audio_st.tmp_pkt))
        {
            printf("!encode audio\n");
            return false;
        }
    }
    //if(current_length < length)
        //return write_audio(buffer + current_length, length - current_length, timestamp + current_length * 1000000 / (2*c->sample_rate));
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
