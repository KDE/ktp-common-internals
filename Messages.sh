#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui" | grep -v '/tests/' | grep -v '/tools/'` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" | grep -v '/tests/' | grep -v '/tools/'` -o $podir/ktp-common-internals.pot
rm -f rc.cpp
