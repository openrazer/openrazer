openrazer-daemon(8)

# NAME

openrazer-daemon - Daemon to manage Razer devices in userspace

# SYNOPSIS

*openrazer-daemon* [options...]

# DESCRIPTION

*openrazer-daemon* is a userspace service which is designed to be an intermediary between userspace programs and the Razer drivers. It will provide some higher level functions to allow similar operation to the Windows Razer Synapse program. This service is designed to be started by xdg-autostart as a session service.

The service has the functionality to sync effects between multiple devices, perform on-the-fly macro recording and playback and reimplements the FN+Keys logic so they work when *macro_keys* are enabled. The service can also perform various ripple effects and turn off the devices when the screensaver is active.

Note, that all paths shown as defaults, eg ~/.local/share/ or ~/.config/ can be changed with XDG Base Directory specification variables: *$XDG_CONFIG_HOME*, *$XDG_DATA_HOME* and *$XDG_RUNTIME_DIR*.

# OPTIONS

*-v*, *--verbose*
	Enable logging of DEBUG messages.

*-F*, *--foreground*
	Run the daemon in the foreground and don't fork. This also enables sending the log output to stdout (in addition to the log file).

*-r*, *--respawn*
	Stop any running daemon first (if one is running) before starting.

*-s*, *--stop*
	Gracefully stop the running daemon.

*--as-root*
	Allow the daemon to be started as root.

*--config*=_config\_file_
	Specifies the location of the config file. If this is not provided it will default to ~/.config/openrazer/razer.conf and create it if needed from the example config.

*--persistence*=_persistence\_file_
	Specifies the location of the persistence file. This will be created if non-existent. This file will store device states so they persist between reboots.

*--run-dir*=_run\_directory_
	Tells the daemon what directory is its run directory, the directory it will change to once started. It will default to *$XDG_RUNTIME_DIR*, if not set it falls back to ~/.local/share/openrazer/.

*--log-dir*=_log\_directory_
	This argument decides where the log directory will be, the daemon itself will handle log rotation as it's a user session service. The daemon will default to ~/.local/share/openrazer/logs/ for its log directory.

*--test-dir*=_test\_dir_
	If provided the daemon will operate in test-driver mode in which it exposes devices that aren't physically connected. Use _scripts/create\_fake\_device.py_ or _scripts/setup\_fake\_devices.sh_ from the source repository to create the directory structure.

# DOCUMENTATION

The full and most up-to-date documentation can be found on our GitHub repository at https://github.com/openrazer/openrazer.

# SEE ALSO

*razer.conf*(5)
