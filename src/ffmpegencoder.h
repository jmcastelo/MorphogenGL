/*
*  Copyright 2021 Jose Maria Castelo Ares
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

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
}
#include <cstdlib>
#include <iostream>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>

#pragma once

class FFmpegEncoder : protected QOpenGLExtraFunctions
{
public:
    unsigned int frameNumber = 0;

    FFmpegEncoder(const char* filename, int width, int height, int fps, const char* preset, const char* crf, QOpenGLContext* mainContext, GLuint id);
    ~FFmpegEncoder();

    void setFrameYUVFromRGB();
    void encodeFrame(AVFrame* oneFrame);
    void recordFrame();
    void setTextureID(GLuint id) { textureID = id; }

private:
    AVCodecContext* codecContext = NULL;
    AVOutputFormat* outputFormat = NULL;
    AVFormatContext* formatContext = NULL;
    AVStream* videoStream = NULL;
    SwsContext* swsContext = NULL;
    AVFrame* frame = NULL;
    AVPacket *packet = NULL;
    uint8_t* pixels = NULL;
    int width;
    int height;
    QOpenGLContext* context;
    GLuint textureID = 0;
};
