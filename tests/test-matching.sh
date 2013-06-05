#!/bin/bash
#

# gedit --gtk-module `pwd`/lib/libautopilot.so &
# GEDIT_PID=$!
# sleep 2

EMPTY_SET='[Argument: a(sv) {}]'
RES=1

RET=`qdbus --literal org.gnome.gedit /com/canonical/Autopilot/Introspection com.canonical.Autopilot.Introspection.GetState '/Root//GtkWindow[opacity=1]'`
if [ "$RET" == "$EMPTY_SET" ]; then
    echo "FAIL"
    RES=0
fi

RET=`qdbus --literal org.gnome.gedit /com/canonical/Autopilot/Introspection com.canonical.Autopilot.Introspection.GetState '/Root//GtkWindow/GtkMenu/GtkImageMenuItem[name=BookmarkOpen]'`
if [ "$RET" == "$EMPTY_SET" ]; then
    echo "FAIL"
    RES=0
fi


# kill $GEDIT_PID
exit $RES
