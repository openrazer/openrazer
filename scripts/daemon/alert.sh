#!/bin/bash

if [ "$1" == "" ]; then
	echo "Keyboard Critical Alert Tool"
	echo "alert.sh action [options...]"
	echo -e "\t Alert actions:"
	echo -e "\t install_ng - Install the alert using syslog-ng"
fi

if [ "$1" == "install_ng" ]; then
	cat <<EOT >> ./tmp.conf
	filter r_notice { level(notice); };
	destination r_key { pipe("/dev/key_alert"); };
	log { source(s_src); filter(r_notice); destination(r_key); };
EOT
	sudo mv tmp.conf /etc/syslog-ng/conf.d/razer_alert.conf
	sudo chown root:root /etc/syslog-ng/conf.d/razer_alert.conf
	sudo service syslog-ng restart
fi

if [ "$1" == "run" ]; then

RUID=( `razer_bcd_controller -C 2 "Breathing Node" "alert node"` )
#RUID=${RUID[1]}
#OUID=`dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.frame_buffer.get | jq '.uid'`
#dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID org.voyagerproject.razer.daemon.render_node.limit_render_time_ms.set int32:1000
#dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$WUID org.voyagerproject.razer.daemon.render_node.next.set int32:$OUID
#dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID org.voyagerproject.razer.daemon.render_node.limit_render_time_ms.set int32:1000
razer_bcd_controller -L $RUID 1000
#dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage.set int32:0

tail -f /dev/key_alert | while read line
do
	echo "$line"
	#OUID=( `dbus-send --system --type=method_call --print-reply=literal --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.frame_buffer.get` )
	OUID=( `razer_bcd_controller -a` )
	#OUID=${OUID[1]}
	echo "actual render_node:$OUID"
	#dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon /$RUID org.voyagerproject.razer.daemon.render_node.next.set int32:$OUID
	if [ $OUID != $RUID ]; then
		razer_bcd_controller -y $RUID $OUID
		#dbus-send --system --type=method_call --dest=org.voyagerproject.razer.daemon / org.voyagerproject.razer.daemon.frame_buffer.connect int32:$RUID
		razer_bcd_controller -b $RUID
	fi
done 
fi

if [ "$1" == "uninstall" ]; then
	sudo rm -f /etc/syslog-ng/conf.d/razer_alert.conf
	sudo rm -f /dev/key_alert
fi