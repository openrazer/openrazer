#!/bin/bash

#dependency : jq
#sudo apt-get install jq


# to retrieve an effect list
#sudo dbus-send --type=method_call --print-reply --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.fx.list

#first parameter is the effect uid
#returns the uid of the render node created as json
RUID=`sudo dbus-send --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_node.create int32:1 string:"Ext Node" string:"a test node" | jq '.uid'`
RUID2=`sudo dbus-send --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_node.create int32:2 string:"Ext Node 2" string:"test node 2" | jq '.uid'`
RUID3=`sudo dbus-send --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_node.create int32:3 string:"Ext Node 3" string:"test node 3" | jq '.uid'`
#[ $? -eq 0 ] || echo "no daemon found";exit $?;
echo "setting render node uid : $RUID"

RNS=`sudo dbus-send --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_nodes.list`

echo "list of render nodes available:\n$RNS"

#set render node to compute/update by uid (deprecated)
#sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.render_node.set int32:$RUID

#connect the frame buffer to the render node by uid
sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.frame_buffer.connect int32:$RUID


#set render node end counter parameter
#sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID/0 org.voyagerproject.razer.daemon.render_node.parameter.set int32:20
#set render node counter parameter
#sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID/1 org.voyagerproject.razer.daemon.render_node.parameter.set int32:0

#set render node dir parameter (increase step)
#sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID/2 org.voyagerproject.razer.daemon.render_node.parameter.set int32:4
#sleep 5
#set render node dir parameter (step back to normal)
#sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID/2 org.voyagerproject.razer.daemon.render_node.parameter.set int32:-1


sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID2 org.voyagerproject.razer.daemon.render_node.input.connect int32:$RUID3
sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID org.voyagerproject.razer.daemon.render_node.input.connect int32:$RUID2

sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID org.voyagerproject.razer.daemon.render_node.opacity.set double:0.2
sudo dbus-send --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID2 org.voyagerproject.razer.daemon.render_node.opacity.set double:0.8
