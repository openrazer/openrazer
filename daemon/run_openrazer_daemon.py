#!/usr/bin/env python3

import argparse
import os
import shutil
import signal
import sys
import time
import contextlib

from openrazer_daemon.daemon import RazerDaemon
from subprocess import check_output
from time import sleep
from daemonize import Daemonize

# Basically copied from https://github.com/jleclanche/python-xdg/blob/master/xdg/basedir.py
HOME = os.path.expanduser("~")
XDG_DATA_HOME = os.environ.get("XDG_DATA_HOME", os.path.join(HOME, ".local", "share"))
XDG_CONFIG_HOME = os.environ.get("XDG_CONFIG_HOME", os.path.join(HOME, ".config"))

RAZER_DATA_HOME = os.path.join(XDG_DATA_HOME, "openrazer")
XDG_RUNTIME_DIR = os.environ.get("XDG_RUNTIME_DIR", RAZER_DATA_HOME)
RAZER_CONFIG_HOME = os.path.join(XDG_CONFIG_HOME, "openrazer")
RAZER_RUNTIME_DIR = XDG_RUNTIME_DIR

EXAMPLE_CONF_FILE = '/usr/share/openrazer/razer.conf.example'

CONF_FILE = os.path.join(RAZER_CONFIG_HOME, 'razer.conf')
LOG_PATH = os.path.join(RAZER_DATA_HOME, 'logs')

args = None

def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose logging')

    parser.add_argument('-F', '--foreground', action='store_true', help='Don\'t fork stay in the foreground')

    parser.add_argument('-r', '--respawn', action='store_true', help='Stop any existing daemon first, if one is running.')
    parser.add_argument('-s', '--stop', action='store_true', help='Gracefully stop the existing daemon.')

    parser.add_argument('--config', type=str, help='Location of the config file', default=CONF_FILE)
    parser.add_argument('--run-dir', type=str, help='Location of the run directory', default=RAZER_RUNTIME_DIR)
    parser.add_argument('--log-dir', type=str, help='Location of the log directory', default=LOG_PATH)

    parser.add_argument('--test-dir', type=str, help='Directory containing test driver structure')

    return parser.parse_args()


def stop_daemon(args):
    pidfile = os.path.join(args.run_dir, 'openrazer-daemon.pid')
    try:
        with open(pidfile) as f:
            pid = int(f.readline().strip())

            # if we have psutil, check that the process name matches the
            # pidfile. Otherwise we might terminate a process that's not
            # ours.
            try:
                import psutil
                try:
                    if psutil.Process(pid).name() != "openrazer-daemon":
                        raise ProcessLookupError()
                except psutil.NoSuchProcess:
                    raise ProcessLookupError()
            except ModuleNotFoundError:
                print("Module psutil is missing, not checking for process name")

            os.kill(pid, signal.SIGTERM)
            pid_exists = True
            delay = 3000
            while delay > 0:
                delay -= 100
                try:
                    time.sleep(0.1)
                    os.kill(pid, 0)
                except ProcessLookupError:
                    print("Process {} stopped".format(pid))
                    pid_exists = False
                    break

            # if we have to kill it, we probably need to remove the
            # pidfile too, otherwise we rely on it to clean up properly
            if pid_exists:
                print("Process {} is hung, sending SIGKILL".format(pid))
                os.kill(pid, signal.SIGKILL)
                os.remove(pidfile)

    except FileNotFoundError:
        print("No pidfile found, assuming openrazer-daemon is not running")
    except ProcessLookupError:
        print("pidfile exists but no process is running. Remove {} and continue".format(pidfile))


def install_example_config_file():
    """
    Installs the example config file in $XDG_CONFIG_HOME/openrazer/razer.conf
    """
    if os.path.exists(CONF_FILE):
        return

    try:
        os.makedirs(os.dirname(CONF_FILE), exist_ok=True)
        if os.path.exists(EXAMPLE_CONF_FILE):
            shutil.copy(EXAMPLE_CONF_FILE, CONF_FILE)
        else:
            print('Cant find "{0}"'.format(EXAMPLE_CONF_FILE), file=sys.stderr)
    except NotADirectoryError as e:
        print("Failed to create {}".format(e.filename), file=sys.stderr)
        sys.exit(1)


def run_daemon():
    global args
    daemon = RazerDaemon(verbose=args.verbose,
                         log_dir=args.log_dir,
                         console_log=args.foreground,
                         config_file=args.config,
                         test_dir=args.test_dir)
    try:
        daemon.run()
    except KeyboardInterrupt:
        daemon.logger.debug("Exited on user request")
    except Exception as err:
        daemon.logger.exception("Caught exception", exc_info=err)


def run():
    global args
    args = parse_args()
    if args.stop:
        stop_daemon(args)
        sys.exit(0)

    if args.respawn:
        stop_daemon(args)
        time.sleep(3)

    install_example_config_file()

    os.makedirs(args.run_dir, exist_ok=True)
    daemon = Daemonize(app="openrazer-daemon",
                       pid=os.path.join(args.run_dir, "openrazer-daemon.pid"),
                       action=run_daemon,
                       foreground=args.foreground,
                       verbose=args.verbose,
                       chdir=args.run_dir)
    daemon.start()

if __name__ == "__main__":
    run()
