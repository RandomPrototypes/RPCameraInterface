#include <RPCameraInterface/VideoEncoder.h>
#include <RPCameraInterface/ImageFormatConverter.h>
#include <stdio.h>

namespace RPCameraInterface
{

bool encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
    int ret;

    /* send the frame to the encoder */
    if (frame)
        ;//printf("Send frame %ld\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        printf("Error sending a frame for encoding\n");
        return false;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            //printf("AVERROR(EAGAIN), AVERROR_EOF\n");
            return true;
        }
        else if (ret < 0) {
            printf("Error during encoding\n");
            return false;
        }

        //printf("Write packet %ld (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }

    return true;
}

class RPCAM_EXPORTS H264EncoderImpl : public H264Encoder
{
public:
	H264EncoderImpl();
	virtual ~H264EncoderImpl();
    virtual bool open(const char *filename, int height, int width, int fps = 30, int bitrate = 2000000, const char *preset = "fast");
    virtual bool write(const std::shared_ptr<ImageData>& img);
    virtual void release();
    virtual void setUseFrameTimestamp(bool useFrameTimestamp);
private:
    const AVCodec *codec;
    AVCodecContext *codecContext;
    struct SwsContext *sws_context;
    AVPacket *pkt;
    FILE *file;
    AVFrame *frame;
    int nbEncodedFrames;
    bool useFrameTimestamp;
    int fps;
    uint64_t firstFrameTimestamp;
};

H264Encoder::~H264Encoder()
{
}

H264EncoderImpl::H264EncoderImpl()
{
	codecContext = NULL;
	sws_context = NULL;
    useFrameTimestamp = false;
    fps = 30;
}

H264EncoderImpl::~H264EncoderImpl()
{
	if(codecContext != NULL)
		release();
}

void H264EncoderImpl::setUseFrameTimestamp(bool useFrameTimestamp)
{
    this->useFrameTimestamp = useFrameTimestamp;
}

bool H264EncoderImpl::open(const char *filename, int height, int width, int fps, int bitrate, const char *preset)
{
    nbEncodedFrames = 0;
    this->fps = fps;
    codec = avcodec_find_encoder(AV_CODEC_ID_H264);//avcodec_find_encoder_by_name("mpeg4");
    if (!codec) {
        printf("Codec not found\n");
        return false;
    }

    codecContext = avcodec_alloc_context3(codec);

    if (!codecContext) {
        printf("Could not allocate video codec context\n");
        return false;
    }

    pkt = av_packet_alloc();
    if (!pkt)
        return false;

    /* put sample parameters */
    codecContext->codec_id = codec->id;
    codecContext->bit_rate = bitrate;
    /* resolution must be a multiple of two */
    codecContext->width = width;
    codecContext->height = height;
    /* frames per second */
    codecContext->time_base.num = 1000;
    codecContext->time_base.den  = 1000*fps;
    codecContext->framerate.num = fps;
    codecContext->framerate.den = 1;
    codecContext->gop_size = 10; /* emit one intra frame every ten frames */
    codecContext->max_b_frames=1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    //H264 only
    av_opt_set(codecContext->priv_data, "preset", preset, 0);

    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        printf("Could not open codec\n");
        return false;
    }

    file = fopen(filename, "wb");
    if (!file) {
        printf("Could not open %s\n", filename);
        return false;
    }

    frame = av_frame_alloc();
    if (!frame) {
        printf("Could not allocate video frame\n");
        return false;
    }
    frame->format = codecContext->pix_fmt;
    frame->width  = codecContext->width;
    frame->height = codecContext->height;
    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    int ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        printf("Could not allocate the video frame data\n");
        return false;
    }
    return true;
}

bool H264EncoderImpl::write(const std::shared_ptr<ImageData>& img)
{
    int ret = av_frame_make_writable(frame);
    if (ret < 0)
        return false;

    const int in_linesize[1] = { 3 * codecContext->width };
    sws_context = sws_getCachedContext(sws_context,
                codecContext->width, codecContext->height, ImageTypeToAVPixelFormat(img->getImageFormat().type),
                codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,
                0, 0, 0, 0);
    unsigned char *data = img->getDataPtr();
    sws_scale(sws_context, (const uint8_t * const *)&data, in_linesize, 0,
            codecContext->height, frame->data, frame->linesize);

    if(useFrameTimestamp) {
        if(nbEncodedFrames == 0)
            firstFrameTimestamp = img->getTimestamp();
        frame->pts = (img->getTimestamp() - firstFrameTimestamp) * fps;
    } else {
        frame->pts = nbEncodedFrames * 1000;
    }

    if(!encode(codecContext, frame, pkt, file))
    {
        printf("!encode\n");
        return false;
    }

    nbEncodedFrames++;
    return true;
}

void H264EncoderImpl::release()
{

    /* flush the encoder */
    encode(codecContext, NULL, pkt, file);

    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
        fwrite(endcode, 1, sizeof(endcode), file);
    fclose(file);
    avcodec_free_context(&codecContext);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    
    codecContext = NULL;
    sws_context = NULL;
}

RPCAM_EXPORTS H264Encoder *createH264EncoderRawPtr()
{
	return new H264EncoderImpl();
}

RPCAM_EXPORTS void deleteH264EncoderRawPtr(H264Encoder *h264Encoder)
{
	delete h264Encoder;
}

}
