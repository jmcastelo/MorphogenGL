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

CONFIG += qt c++11

QMAKE_CXXFLAGS_RELEASE += /MT

DEFINES += QT_DEPRECATED_WARNINGS

QT += widgets printsupport

INCLUDEPATH += C:/Development/include

LIBS += -LC:/Development/libs \
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

CONFIG(debug, debug|release) {
    LIBS += -lqcustomplotd2
}

CONFIG(release, debug|release) {
    LIBS += -lqcustomplot
}

HEADERS += ./src/blender.h \
    ./src/fbo.h \
    ./src/ffmpegencoder.h \
    ./src/generator.h \
    ./src/imageoperations.h \
    ./src/parameter.h \
    ./src/pipeline.h \
    ./src/seed.h \
    ./src/controlwidget.h \
    ./src/configparser.h \
    ./src/focuslineedit.h \
    ./src/morphowidget.h \
    ./src/operationswidget.h \
    ./src/polarkernelplot.h

SOURCES += ./src/blender.cpp \
    ./src/configparser.cpp \
    ./src/controlwidget.cpp \
    ./src/fbo.cpp \
    ./src/ffmpegencoder.cpp \
    ./src/generator.cpp \
    ./src/imageoperations.cpp \
    ./src/main.cpp \
    ./src/morphowidget.cpp \
    ./src/parameter.cpp \
    ./src/pipeline.cpp \
    ./src/polarkernelplot.cpp \
    ./src/seed.cpp

RESOURCES += resource.qrc

RC_ICONS = ./icons/morphogengl.ico
