#!/bin/bash
##################################
# Author: Juhapekka Piiroinen
# juhapekka.piiroinen@gmail.com
# 2011
# GPLv3
##################################
tar -xvjpf OpenCV-2.2.0.tar.bz2
ln -s OpenCV-2.2.0 opencv
mkdir opencv-build
cd opencv-build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_QT=ON -D WITH_TBB=YES -D WITH_V4L=1 -D TBB_INCLUDE_DIRS=/usr/include/tbb -D BUILD_SAMPLES=ON -D PYTHON_INCLUDE_DIRS=/usr/include/python2.7 ../opencv
make -j`cat /proc/cpuinfo|grep processor|wc -l`
