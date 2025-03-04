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

TARGET = morphogen

DESTDIR = ../../

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
    src/graphwidget.h \
    src/gridwidget.h \
    src/histogramwidget.h \
    src/imageoperation.h \
    src/mainwindow.h \
    src/message.h \
    src/midicontrol.h \
    src/midiwidget.h \
    src/morphowidget.h \
    src/node.h \
    src/nodemanager.h \
    src/operationbuilder.h \
    src/operationwidget.h \
    src/overlay.h \
    src/parameters/number.h \
    src/parameters/optionsparameter.h \
    src/parameters/parameter.h \
    src/parameters/uniformmat4parameter.h \
    src/parameters/uniformparameter.h \
    src/plotswidget.h \
    src/recorder.h \
    src/rgbwidget.h \
    src/seed.h \
    src/texformat.h \
    src/timerthread.h \
    src/widgets/focuswidgets.h \
    src/widgets/layoutformat.h \
    src/widgets/optionswidget.h \
    src/widgets/parameterwidget.h \
    src/widgets/uniformmat4widget.h \
    src/widgets/uniformwidget.h

SOURCES += \
    src/blender.cpp \
    src/colorpath.cpp \
    src/configparser.cpp \
    src/controlwidget.cpp \
    src/cycle.cpp \
    src/cyclesearch.cpp \
    src/edge.cpp \
    src/fbo.cpp \
    src/graphwidget.cpp \
    src/gridwidget.cpp \
    src/histogramwidget.cpp \
    src/imageoperation.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/message.cpp \
    src/midicontrol.cpp \
    src/midiwidget.cpp \
    src/morphowidget.cpp \
    src/node.cpp \
    src/nodemanager.cpp \
    src/operationbuilder.cpp \
    src/operationwidget.cpp \
    src/overlay.cpp \
    src/parameters/optionsparameter.cpp \
    src/parameters/uniformmat4parameter.cpp \
    src/parameters/uniformparameter.cpp \
    src/plotswidget.cpp \
    src/recorder.cpp \
    src/rgbwidget.cpp \
    src/seed.cpp \
    src/timerthread.cpp \
    src/widgets/uniformmat4widget.cpp \
    src/widgets/uniformwidget.cpp
