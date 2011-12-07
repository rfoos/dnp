#!/bin/bash
## Prebuild steps
# @author Rick Foos at SolengTech.com
# Qmake from command line requires path, or link.
# yum -y install qt-devel
# sudo ln -s /usr/lib64/qt4/bin/qmake /usr/bin/qmake
# @file

export QMAKESPEC=freebsd-g++
qmake -project "CONFIG += qtestlib" "QT += network"
qmake
make

