#  Copyright 2020 Jose Maria Castelo Ares
#
#  Contact: <jose.maria.castelo@gmail.com>
#  Repository: <https://github.com/jmcastelo/MorphogenGL>
#
#  This file is part of MorphogenGL.
#
#  MorphogenGL is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  MorphogenGL is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.

TARGET = MorphogenGL

TEMPLATE = app

CONFIG += qt c++14

win32:QMAKE_CXXFLAGS_RELEASE += /MT

DEFINES += QT_DEPRECATED_WARNINGS

QT += widgets printsupport

win32:INCLUDEPATH += C:/Development/include

win32:LIBS += -LC:/Development/libs \
    -lavcodec \
    -lavformat \
    -lswscale \
    -lavutil \
    -lswresample \
    -llibx264 \
    -lbcrypt \
    -lws2_32 \
    -lsecur32 \
    -lkernel32 \
    -luser32 \
    -lgdi32 \
    -lwinspool \
    -lcomdlg32 \
    -ladvapi32 \
    -lshell32 \
    -lole32 \
    -loleaut32 \
    -luuid \
    -lodbc32 \
    -lodbccp32

unix:INCLUDEPATH += /usr/local/include

unix:LIBS += -L/usr/local/lib \
    -lavformat \
    -lavcodec \
    -lswscale \
    -lavutil \
    -lavresample \
    -lx264 \
    -lz \
    -lbz2 \
    -lX11 \
    -lvdpau

CONFIG(debug, debug|release) {
    LIBS += -lqcustomplotd
}

CONFIG(release, debug|release) {
    LIBS += -lqcustomplot
    #CONFIG += link_pkgconfig
    #PKGCONFIG += libavcodec libavformat libavutil libswscale libavresample x264
}

RESOURCES += resource.qrc

RC_ICONS = ./icons/morphogengl.ico

#unix:!macx: LIBS += -L/usr/local/lib -lavcodec -lavformat -lavutil -lswscale -lx264 -lz -lX11 -lbz2

#INCLUDEPATH += /usr/local/include
#DEPENDPATH += /usr/local/include

#unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libavcodec.a
#unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libavformat.a
#unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libavutil.a
#unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libavresample.a
#unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libswscale.a
#unix:!macx: PRE_TARGETDEPS += /usr/local/lib/libx264.a

HEADERS += \
    src/blender.h \
    src/configparser.h \
    src/controlwidget.h \
    src/cycle.h \
    src/cyclesearch.h \
    src/edge.h \
    src/fbo.h \
    src/ffmpegencoder.h \
    src/focuslineedit.h \
    src/generator.h \
    src/graphwidget.h \
    src/heart.h \
    src/imageoperations.h \
    src/mainwidget.h \
    src/morphowidget.h \
    src/node.h \
    src/operationswidget.h \
    src/parameter.h \
    src/plotswidget.h \
    src/polarkernelplot.h \
    src/returnmap.h \
    src/rgbwidget.h \
    src/seed.h

SOURCES += \
    src/blender.cpp \
    src/configparser.cpp \
    src/controlwidget.cpp \
    src/cycle.cpp \
    src/cyclesearch.cpp \
    src/edge.cpp \
    src/fbo.cpp \
    src/ffmpegencoder.cpp \
    src/generator.cpp \
    src/graphwidget.cpp \
    src/heart.cpp \
    src/imageoperations.cpp \
    src/main.cpp \
    src/mainwidget.cpp \
    src/morphowidget.cpp \
    src/node.cpp \
    src/plotswidget.cpp \
    src/polarkernelplot.cpp \
    src/returnmap.cpp \
    src/rgbwidget.cpp \
    src/seed.cpp
