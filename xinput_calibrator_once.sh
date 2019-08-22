#!/bin/bash

BINARY="xinput_calibrator_libinput"
CALFILE="/etc/pointercal.xinput"
LOGFILE="/var/log/xinput_calibrator.pointercal.log"

if [ -e "$CALFILE" ]; then
        if grep "replace" "$CALFILE" ; then
                echo "Empty calibration file found, removing it"
                rm "$CALFILE" 2>/dev/null || true
        else
                echo "Using calibration data stored in $CALFILE"
                . "$CALFILE" && exit 0
        fi
fi

CALDATA="$($BINARY | tee $LOGFILE | grep 'xinput set' | sed 's/^    //g; s/$/;/g')"
if [ -n "$CALDATA" ] ; then
        echo "$CALDATA" > $CALFILE
        echo "Calibration data stored in $CALFILE (log in $LOGFILE)"
fi

source "$CALFILE"
