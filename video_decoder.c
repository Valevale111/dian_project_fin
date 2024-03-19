#include "video_decoder.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct VideoDecoder {
    int status;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    struct SwsContext *sws_ctx;
    int videoStreamIndex;
    AVFrame *pFrameRGB;
    AVPacket *packet;
    AVFrame *pFrame;
    int frameIndex;
    double fps;
    int totalFrames;
} VideoDecoder;
    
static VideoDecoder _video_decoder = {};

int decoder_init(const char *filename) {
    if(_video_decoder.status == 1) {
        fprintf(stderr, "The repeated call to init did not close before\n");
        return -1;
    }

    _video_decoder.pFormatCtx = NULL;
    if (avformat_open_input(&_video_decoder.pFormatCtx, filename, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open input file %s\n", filename);
        return -1;
    }

    if (avformat_find_stream_info(_video_decoder.pFormatCtx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        decoder_close();
        return -1;
    }

    _video_decoder.videoStreamIndex = -1;
    for (int i = 0; i < _video_decoder.pFormatCtx->nb_streams; i++) {
        if (_video_decoder.pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            _video_decoder.videoStreamIndex = i;
            break;
        }
    }
    if (_video_decoder.videoStreamIndex == -1) {
        fprintf(stderr, "Didn't find a video stream\n");
        decoder_close();
        return -1;
    }

    AVCodecParameters *codecParameters = _video_decoder.pFormatCtx->streams[_video_decoder.videoStreamIndex]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
    _video_decoder.pCodecCtx = avcodec_alloc_context3(codec);
    if (!_video_decoder.pCodecCtx) {
        fprintf(stderr, "Failed to allocate codec\n");
        decoder_close();
        return -1;
    }

    if (avcodec_parameters_to_context(_video_decoder.pCodecCtx, codecParameters) < 0) {
        fprintf(stderr, "Failed to copy codec parameters to codec context\n");
        decoder_close();
        return -1;
    }

    if (avcodec_open2(_video_decoder.pCodecCtx, codec, NULL) < 0) {
        fprintf(stderr, "Failed to open codec\n");
        decoder_close();
        return -1;
    }

    _video_decoder.sws_ctx = sws_getContext(
        _video_decoder.pCodecCtx->width, _video_decoder.pCodecCtx->height, _video_decoder.pCodecCtx->pix_fmt,
        _video_decoder.pCodecCtx->width, _video_decoder.pCodecCtx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    if (!_video_decoder.sws_ctx) {
        fprintf(stderr, "Failed to initialize SWS context\n");
        decoder_close();
        return -1;
    }

    _video_decoder.pFrame = av_frame_alloc();
    _video_decoder.pFrameRGB = av_frame_alloc();
    if (!_video_decoder.pFrame || !_video_decoder.pFrameRGB) {
        fprintf(stderr, "Failed to allocate frame\n");
        decoder_close();
        return -1;
    }

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, _video_decoder.pCodecCtx->width, _video_decoder.pCodecCtx->height, 32);
    uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer\n");
        decoder_close();
        return -1;
    }
    av_image_fill_arrays(_video_decoder.pFrameRGB->data, _video_decoder.pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, _video_decoder.pCodecCtx->width, _video_decoder.pCodecCtx->height, 32);

    _video_decoder.packet = av_packet_alloc();
    if (!_video_decoder.packet) {
        fprintf(stderr, "Failed to allocate packet\n");
        decoder_close();
        return -1;
    }

    _video_decoder.frameIndex = 0;

    AVRational time_base = _video_decoder.pFormatCtx->streams[_video_decoder.videoStreamIndex]->time_base;
    AVRational frame_rate = _video_decoder.pFormatCtx->streams[_video_decoder.videoStreamIndex]->avg_frame_rate;
    _video_decoder.fps = av_q2d(frame_rate);

    int64_t duration = _video_decoder.pFormatCtx->streams[_video_decoder.videoStreamIndex]->duration;
    if (duration != AV_NOPTS_VALUE) {
        int64_t total_frames = (int64_t)((double)duration * _video_decoder.fps * av_q2d(time_base));
        _video_decoder.totalFrames = total_frames;
    } else {
        _video_decoder.totalFrames = 0; // Unknown total frames
    }

    _video_decoder.status = 1;
    return 0;
}

Frame decoder_get_frame() {
    Frame frame = {0};
    if(_video_decoder.status == 0) {
        fprintf(stderr, "The decoder is uninitialized\n");
        return frame;
    }

    while (av_read_frame(_video_decoder.pFormatCtx, _video_decoder.packet) >= 0) {
        if (_video_decoder.packet->stream_index == _video_decoder.videoStreamIndex) {
            avcodec_send_packet(_video_decoder.pCodecCtx, _video_decoder.packet);
            if (avcodec_receive_frame(_video_decoder.pCodecCtx, _video_decoder.pFrame) == 0) {
                sws_scale(_video_decoder.sws_ctx, (uint8_t const * const *)_video_decoder.pFrame->data, _video_decoder.pFrame->linesize, 0, _video_decoder.pCodecCtx->height, _video_decoder.pFrameRGB->data, _video_decoder.pFrameRGB->linesize);
                _video_decoder.pFrameRGB->width = _video_decoder.pCodecCtx->width;
                _video_decoder.pFrameRGB->height = _video_decoder.pCodecCtx->height;
                av_packet_unref(_video_decoder.packet);
                _video_decoder.frameIndex++;

                frame.height = _video_decoder.pFrameRGB->height;
                frame.width = _video_decoder.pFrameRGB->width;
                frame.linesize = _video_decoder.pFrameRGB->linesize[0];
                frame.data = _video_decoder.pFrameRGB->data[0];
                return frame;
            }
        }
        av_packet_unref(_video_decoder.packet);
    }
    return frame;
}

void decoder_close() {
    if (_video_decoder.sws_ctx) sws_freeContext(_video_decoder.sws_ctx);
    if (_video_decoder.pFrame) av_frame_free(&_video_decoder.pFrame);
    if (_video_decoder.pFrameRGB) {
        av_freep(&_video_decoder.pFrameRGB->data[0]); // Free the buffer allocated with av_malloc
        av_frame_free(&_video_decoder.pFrameRGB);
    }
    if (_video_decoder.pCodecCtx) avcodec_close(_video_decoder.pCodecCtx);
    if (_video_decoder.pFormatCtx) avformat_close_input(&_video_decoder.pFormatCtx);
    if (_video_decoder.packet) av_packet_free(&_video_decoder.packet);

    _video_decoder.status = 0;
}

double get_fps() {
    if(_video_decoder.status == 0) {
        fprintf(stderr, "The decoder is uninitialized\n");
        return 0;
    }
    return _video_decoder.fps;
}

int get_frame_index() {
    if(_video_decoder.status == 0) {
        fprintf(stderr, "The decoder is uninitialized\n");
        return 0;
    }
    return _video_decoder.frameIndex;
}

int get_total_frames() {
    if(_video_decoder.status == 0) {
        fprintf(stderr, "The decoder is uninitialized\n");
        return 0;
    }
    return _video_decoder.totalFrames;
}
