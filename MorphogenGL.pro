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

CONFIG += qt c++20

QMAKE_CXXFLAGS += -DLIBREMIDI_ALSA=1 -DLIBREMIDI_HEADER_ONLY=1 -pthread

LIBS += -lasound -pthread

QT += widgets openglwidgets multimedia opengl

RESOURCES += resource.qrc

#RC_ICONS = ./icons/morphogengl.ico

HEADERS += \
    src/blender.h \
    src/blendfactorwidget.h \
    src/colorpath.h \
    src/configparser.h \
    src/controlwidget.h \
    src/cycle.h \
    src/cyclesearch.h \
    src/edge.h \
    src/fbo.h \
    src/generator.h \
    src/graphwidget.h \
    src/histogramwidget.h \
    src/imageoperations.h \
    src/mainwindow.h \
    src/message.h \
    src/midicontrol.h \
    src/midiwidget.h \
    src/morphowidget.h \
    src/node.h \
    src/operationswidget.h \
    src/overlay.h \
    src/parameter.h \
    src/plotswidget.h \
    src/recorder.h \
    src/rgbwidget.h \
    src/seed.h \
    src/texformat.h \
    src/timerthread.h \
    src/widgets/arraywidget.h \
    src/widgets/floatwidget.h \
    src/widgets/focuswidgets.h \
    src/widgets/intoddwidget.h \
    src/widgets/intwidget.h \
    src/widgets/kernelwidget.h \
    src/widgets/matrixwidget.h \
    src/widgets/optionswidget.h \
    src/widgets/parameterwidget.h

SOURCES += \
    src/blender.cpp \
    src/colorpath.cpp \
    src/configparser.cpp \
    src/controlwidget.cpp \
    src/cycle.cpp \
    src/cyclesearch.cpp \
    src/edge.cpp \
    src/fbo.cpp \
    src/generator.cpp \
    src/graphwidget.cpp \
    src/histogramwidget.cpp \
    src/imageoperations.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/message.cpp \
    src/midicontrol.cpp \
    src/midiwidget.cpp \
    src/morphowidget.cpp \
    src/node.cpp \
    src/overlay.cpp \
    src/plotswidget.cpp \
    src/recorder.cpp \
    src/rgbwidget.cpp \
    src/seed.cpp \
    src/timerthread.cpp
