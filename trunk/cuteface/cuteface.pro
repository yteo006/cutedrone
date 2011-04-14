# Add files and directories to ship with the application 
# by adapting the examples below.
# file1.source = myfile
# dir1.source = mydir
DEPLOYMENTFOLDERS = # file1 dir1

symbian:TARGET.UID3 = 0xE4BC196E

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
# CONFIG += mobility
# MOBILITY +=

SOURCES += main.cpp mainwindow.cpp \
    facedetect.cpp
HEADERS += mainwindow.h \
    facedetect.h
FORMS += mainwindow.ui

INCLUDEPATH += ../3rdparty/opencv/include/opencv
LIBS += ../3rdparty/opencv-build/lib/libopencv_core.so \
        ../3rdparty/opencv-build/lib/libopencv_highgui.so \
        ../3rdparty/opencv-build/lib/libopencv_objdetect.so

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

RESOURCES +=
