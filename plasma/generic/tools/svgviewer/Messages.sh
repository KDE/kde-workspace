#! /usr/bin/env bash
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp *.h -o $podir/plasmasvgviewer.pot
rm -f rc.cpp

