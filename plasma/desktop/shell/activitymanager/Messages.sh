#! /usr/bin/env bash
$XGETTEXT `find . -name \*.qml` -L Java -o $podir/plasma_package_org.kde.desktop.activitymanager.pot
$XGETTEXT `find . -name \*.cpp` -j -o $podir/plasma_package_org.kde.desktop.activitymanager.pot
rm -f rc.cpp
