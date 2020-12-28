/*
*  Copyright 2020 José María Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/MorphogenGL>
*
*  This file is part of MorphogenGL.
*
*  MorphogenGL is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  MorphogenGL is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "ffmpegencoder.h"

FFmpegEncoder::FFmpegEncoder(const char* filename, int theWidth, int theHeight, int fps, const char* preset, const char* crf, QOpenGLContext* mainContext) : width { theWidth }, height { theHeight }, context { mainContext }
{
    initializeOpenGLFunctions();

    av_register_all();

    // Init encoding format

    outputFormat = av_guess_format(NULL, filename, NULL);
    if (!outputFormat)
        outputFormat = av_guess_format("mpeg4", NULL, NULL);
    if (!outputFormat)
        throw "Could not find suitable output format\n";

    outputFormat->video_codec = AV_CODEC_ID_H264;

    // Init codec

    AVCodec* codec = NULL;
    codec = avcodec_find_encoder(outputFormat->video_codec);
    if (!codec)
        throw "Codec not found\n";

    // Init codec context

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
        throw "Could not allocate codec context\n";

    codecContext->codec_id = outputFormat->video_codec;
    codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    codecContext->bit_rate = 4 * width * height;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->time_base.num = 1;
    codecContext->time_base.den = fps;
    codecContext->max_b_frames = 1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    // H.264 specific options

    codecContext->gop_size = 25;
    codecContext->level = 31;

    av_opt_set(codecContext->priv_data, "preset", preset, 0);
    av_opt_set(codecContext->priv_data, "crf", crf, 0);

    // Init format context

    formatContext = avformat_alloc_context();
    if (!formatContext)
        throw "Could not allocate format context\n";
    formatContext->oformat = outputFormat;
    formatContext->video_codec_id = outputFormat->video_codec;
    
    snprintf(formatContext->filename, sizeof(formatContext->filename), "%s", filename);

    av_dict_set(&formatContext->metadata, "comment", "MorphogenGL", 0);

    // Init stream

    videoStream = avformat_new_stream(formatContext, codec);
    if (!videoStream)
        throw "Could not allocate stream\n";
    videoStream->codec = codecContext;
    videoStream->time_base.num = 1;
    videoStream->time_base.den = fps;

    // Set global header flags

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
        codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // Open codec context

    if (avcodec_open2(codecContext, codec, NULL) < 0)
        throw "Could not open codec\n";

    // Open output file

    if (!(outputFormat->flags & AVFMT_NOFILE))
        if (avio_open(&formatContext->pb, filename, AVIO_FLAG_READ_WRITE) < 0)
            throw "Could not open output file\n";

    // Write header

    avformat_write_header(formatContext, NULL);

    // Dump AV format information

    av_dump_format(formatContext, 0, filename, 1);

    // Init frame

    frame = av_frame_alloc();
    if (!frame)
        throw "Could not allocate video frame\n";

    av_frame_make_writable(frame);

    frame->format = codecContext->pix_fmt;
    frame->width = codecContext->width;
    frame->height = codecContext->height;

    if (av_frame_get_buffer(frame, 32) < 0)
        throw "Could not allocate output frame buffer\n";

    // Init packet

    packet = av_packet_alloc();
    if (!packet)
        throw "Could not allocate packet\n";

    // Init frame number

    frameNumber = 0;

    // Allocate pixels array

    pixels = new uint8_t[4 * width * height];
}

FFmpegEncoder::~FFmpegEncoder()
{
    // Write pending packets

    encodeFrame(NULL);

    // Write file trailer

    av_write_trailer(formatContext);

    // Close file and free resources

    avio_close(formatContext->pb);

    avformat_free_context(formatContext);

    av_frame_free(&frame);
    
    av_packet_free(&packet);

    sws_freeContext(swsContext);

    delete pixels;
}

void FFmpegEncoder::setFrameYUVFromRGB()
{
    context->makeCurrent(context->surface());
    glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    context->doneCurrent();

    const int inLinesize[1] = { 4 * codecContext->width };

    swsContext = sws_getCachedContext(swsContext, codecContext->width, codecContext->height, AV_PIX_FMT_RGB32, codecContext->width, codecContext->height, AVPixelFormat::AV_PIX_FMT_YUV420P, 0, NULL, NULL, NULL);
    sws_scale(swsContext, (const uint8_t* const*)&pixels, inLinesize, 0, codecContext->height, frame->data, frame->linesize);
}

void FFmpegEncoder::encodeFrame(AVFrame* oneFrame)
{
    setFrameYUVFromRGB();

    if (avcodec_send_frame(codecContext, oneFrame) < 0)
        throw "Error sending a frame for encoding\n";

    int ret = 0;

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codecContext, packet);
    
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
            throw "Error during encoding\n";
        
        av_packet_rescale_ts(packet, codecContext->time_base, videoStream->time_base);
        packet->stream_index = videoStream->index;
        
        if (av_write_frame(formatContext, packet) < 0)
            throw "Error writing frame\n";
        
        av_packet_unref(packet);
    }
}

void FFmpegEncoder::recordFrame()
{
    frame->pts = frameNumber++;
    try
    {
        encodeFrame(frame);
    }
    catch (const char* exception)
    {
        std::cout << exception << std::endl;
    }
}