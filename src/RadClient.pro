#-------------------------------------------------
#
# Project created by QtCreator 2016-01-29T09:18:31
#
#-------------------------------------------------

QT       += core gui network widgets

QMAKE_CXXFLAGS += -std=c++11

CONFIG  += console
TARGET   = RadClient
TEMPLATE = app

SOURCES += src/main.cpp \
           src/radclient.cpp \
           src/socketclient.cpp \
           src/historylineedit.cpp \
           src/minimap.cpp

HEADERS  += inc/radclient.h \
            inc/socketclient.h \
            inc/historylineedit.h \
            inc/minimap.h

LIBS += -lz

FORMS    += radclient.ui

RESOURCES += RadClient.qrc
