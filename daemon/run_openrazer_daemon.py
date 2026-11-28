#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0-or-later

import argparse
import os
import shutil
import sys

from openrazer_daemon.daemon import RazerDaemon, __version__

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
PERSISTENCE_FILE = os.path.join(RAZER_CONFIG_HOME, 'persistence.conf')
LOG_PATH = os.path.join(RAZER_DATA_HOME, 'logs')


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose logging')

    parser.add_argument('--version', action='version', version='%(prog)s {version}'.format(version=__version__))

    parser.add_argument('--as-root', action='store_true', help='Allow the daemon to be started as root')

    parser.add_argument('--config', type=str, help='Location of the config file', default=CONF_FILE)
    parser.add_argument('--persistence', type=str, help='Location to file for storing device persistence data', default=PERSISTENCE_FILE)
    parser.add_argument('--run-dir', type=str, help='Location of the run directory', default=RAZER_RUNTIME_DIR)
    parser.add_argument('--log-dir', type=str, help='Location of the log directory', default=LOG_PATH)

    parser.add_argument('--test-dir', type=str, help='Directory containing test driver structure')

    return parser.parse_args()


def install_example_config_file(config_file):
    """
    Installs the example config file
    """
    if os.path.exists(config_file):
        return

    try:
        os.makedirs(os.path.dirname(config_file), exist_ok=True)
        if os.path.exists(EXAMPLE_CONF_FILE):
            shutil.copy(EXAMPLE_CONF_FILE, config_file)
        else:
            print('Cant find "{0}"'.format(EXAMPLE_CONF_FILE), file=sys.stderr)
    except NotADirectoryError as e:
        print("Failed to create {}".format(e.filename), file=sys.stderr)
        sys.exit(1)


def init_persistence_config(persistence_file):
    """
    Creates a new file for persistence, if it does not exist.
    """
    if os.path.exists(persistence_file):
        return

    try:
        os.makedirs(os.path.dirname(persistence_file), exist_ok=True)
        with open(persistence_file, "w") as f:
            f.writelines("")

    except NotADirectoryError as e:
        print("Failed to create {}".format(e.filename), file=sys.stderr)
        sys.exit(1)


def run():
    args = parse_args()

    if os.getuid() == 0:
        if args.as_root:
            print("The daemon is being run as root.")
        else:
            print("The daemon should not be run as root. If you have a good reason to do so, use the --as-root flag.")
            sys.exit(1)

    install_example_config_file(args.config)
    init_persistence_config(args.persistence)

    os.makedirs(args.run_dir, exist_ok=True)
    daemon = RazerDaemon(verbose=args.verbose,
                         log_dir=args.log_dir,
                         config_file=args.config,
                         persistence_file=args.persistence,
                         test_dir=args.test_dir)
    try:
        daemon.run()
    except KeyboardInterrupt:
        daemon.logger.debug("Exited on user request")
    except Exception as err:
        daemon.logger.exception("Caught exception", exc_info=err)


if __name__ == "__main__":
    run()
