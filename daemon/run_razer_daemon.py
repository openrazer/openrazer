#!/usr/bin/env python3

import argparse
import os
import shutil
import signal
import sys

from razer_daemon.daemon import daemonize
from subprocess import check_output
from time import sleep

# Basically copied from https://github.com/jleclanche/python-xdg/blob/master/xdg/basedir.py
HOME = os.path.expanduser("~")
XDG_DATA_HOME = os.environ.get("XDG_DATA_HOME", os.path.join(HOME, ".local", "share"))
XDG_CONFIG_HOME = os.environ.get("XDG_CONFIG_HOME", os.path.join(HOME, ".config"))

RAZER_DATA_HOME = os.path.join(XDG_DATA_HOME, "razer-daemon")
XDG_RUNTIME_DIR = os.environ.get("XDG_RUNTIME_DIR", RAZER_DATA_HOME)
RAZER_CONFIG_HOME = os.path.join(XDG_CONFIG_HOME, "razer-daemon")
RAZER_RUNTIME_DIR = os.path.join(XDG_RUNTIME_DIR, "razer-daemon")

EXAMPLE_CONF_FILE = '/usr/share/razer-daemon/razer.conf.example'

CONF_FILE = os.path.join(RAZER_CONFIG_HOME, 'razer.conf')
LOG_PATH = os.path.join(RAZER_DATA_HOME, 'logs')


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


def stop_daemon():
    try:
        pid = int(check_output(["pidof", "-s", "razer-daemon"]))
        print("Stopping razer-daemon... (PID {0})".format(str(pid)))
        os.kill(pid, signal.SIGTERM)
        sleep(3)

        # Give it time to stop
        pid = check_output(["pidof", "-s", "razer-daemon"])
        if len(pid) > 0:
            os.kill(int(pid), signal.SIGKILL)
            sleep(3)

    except Exception:
        print("No razer-daemon currently running.")


def run():
    args = parse_args()

    # TODO Fix up run_dir (especially in macros branch as things will break)
    if not os.path.exists(RAZER_CONFIG_HOME):
        os.makedirs(RAZER_CONFIG_HOME, exist_ok=True)
    if not os.path.exists(RAZER_DATA_HOME):
        os.makedirs(RAZER_DATA_HOME, exist_ok=True)
    if not os.path.exists(LOG_PATH):
        os.makedirs(LOG_PATH, exist_ok=True)
    if not os.path.exists(CONF_FILE):
        if os.path.exists(EXAMPLE_CONF_FILE):
            shutil.copy(EXAMPLE_CONF_FILE, CONF_FILE)
        else:
            print('Cant find "{0}"'.format(EXAMPLE_CONF_FILE), file=sys.stderr)

    if args.stop:
        stop_daemon()
        sys.exit(0)

    if args.respawn:
        stop_daemon()

    daemon_args = {
        'verbose': args.verbose,
        'foreground': args.foreground,
        'log_dir': args.log_dir,
        'test_dir': args.test_dir
    }

    if args.foreground:
        daemon_args['console_log'] = True

    if args.config:
        daemon_args['config_file'] = args.config

    if args.run_dir:
        daemon_args['run_dir'] = args.run_dir

    daemonize(**daemon_args)





run()
