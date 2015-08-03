#!/bin/bash

#dependency : jq
#sudo apt-get install jq


dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.fx.lib.load string:"daemon/fx/pez2001_mixer_debug.so"

RUID=`dbus-send --system --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_node.create int32:10 string:"Light Blast" string:"heatmap alike" | jq '.uid'`
FUID=`dbus-send --system --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_node.create int32:6 string:"2nd fx" string:"testing" | jq '.uid'`
GUID=`dbus-send --system --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_node.create int32:17 string:"Glimmer" string:"glimming mixer" | jq '.uid'`


dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$GUID org.voyagerproject.razer.daemon.render_node.input.connect int32:$RUID
#dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$GUID org.voyagerproject.razer.daemon.render_node.second_input.connect int32:$FUID
dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$GUID org.voyagerproject.razer.daemon.render_node.opacity.set double:0.2

dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.frame_buffer.connect int32:$GUID
dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.fps.set int32:8



