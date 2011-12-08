#!/bin/bash
## Prebuild steps
# @author Rick Foos at SolengTech.com
# Qmake from command line requires path, or link.
# Fedora/RHEL/CentOS
# yum -y groupinstgall "Development Tools"
# yum -y install qt-devel
# sudo ln -s /usr/lib64/qt4/bin/qmake /usr/bin/qmake
# On Ubuntu, qmake is on the path.
# apt-get install build-essential git-core qt4-qmake qt4-dev-tools
# @file

export QMAKESPEC=freebsd-g++
qmake -project "CONFIG += qtestlib" "QT += network"
qmake
make

[ -x "./dnp" ] && echo "Now run ./dnp"
