#! /usr/bin/env bash
$EXTRACTRC *.ui  >> rc.cpp || exit 11
$XGETTEXT *.cpp */*.cpp *.h -o $podir/libqmltaskmanager.pot
rm -f rc.cpp
