#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/plasma_applet_tasks.pot
$XGETTEXT `find . -name \*.qml` -L Java --join -o $podir/plasma_applet_tasks.pot
rm -f rc.cpp
