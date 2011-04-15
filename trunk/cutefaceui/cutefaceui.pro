#-------------------------------------------------
#
# Project created by QtCreator 2011-04-15T22:44:40
#
#-------------------------------------------------

QT       += core gui network

TARGET = cutefaceui
TEMPLATE = app
target.path=/usr/local/bin
INSTALLS=target


SOURCES += main.cpp\
        mainwindow.cpp \
        ../src/cutedrone.cpp

HEADERS  += mainwindow.h \
../src/cutedrone.h

FORMS    += mainwindow.ui

AR_DRONE_SDK= $$(HOME)/ARDrone_SDK_1_6_20110224
LIBS +=-L$${AR_DRONE_SDK}/lib
LIBS += $${AR_DRONE_SDK}/lib/libvlib.a
LIBS += $${AR_DRONE_SDK}/lib/libsdk.a
LIBS += $${AR_DRONE_SDK}/lib/libpc_ardrone.a


INCLUDEPATH += $${AR_DRONE_SDK}/ARDroneLib/Soft/Common \
               $${AR_DRONE_SDK}/ARDroneLib/VP_SDK \
               $${AR_DRONE_SDK}/ARDroneLib/VP_SDK/VP_Os/linux \
               $${AR_DRONE_SDK}/ARDroneLib/VP_SDK/VP_Os \
               $${AR_DRONE_SDK}/ARDroneLib/Soft/Lib \
               $${AR_DRONE_SDK}/ARDroneLib/VLIB \
               $${AR_DRONE_SDK}/ARDroneLib
