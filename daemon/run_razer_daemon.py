#!/usr/bin/env python3

import argparse
import os
import shutil
import sys
from razer_daemon.daemon import daemonize

SHARE_DIR = '/usr/share/razer-service'
EXAMPLE_CONF = os.path.join(SHARE_DIR, 'razer.conf.example')

BASE_PATH = os.path.join(os.path.abspath(os.environ['HOME']), '.razer-service')
CONF_PATH = os.path.join(BASE_PATH, 'razer.conf')
LOG_PATH = os.path.join(BASE_PATH, 'logs')


def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose logging')

    parser.add_argument('-F', '--foreground', action='store_true', help='Don\'t fork stay in the foreground')

    parser.add_argument('--config', type=str, help='Location of the config file', default=CONF_PATH)
    parser.add_argument('--run-dir', type=str, help='Location of the data directory', default=BASE_PATH)
    parser.add_argument('--log-dir', type=str, help='Location of the log directory', default=LOG_PATH)
    parser.add_argument('--pid-file', type=str, help='Location of the pid file')

    parser.add_argument('--test-dir', type=str, help='Directory containing test driver structure')

    return parser.parse_args()


def run():
    args = parse_args()

    if not os.path.exists(BASE_PATH):
        os.mkdir(BASE_PATH)
        os.mkdir(LOG_PATH)
        os.mkdir(os.path.join(BASE_PATH, 'data'))
        if os.path.exists(EXAMPLE_CONF):
            shutil.copy(EXAMPLE_CONF, CONF_PATH)
        else:
            print('Cant find "{0}"'.format(EXAMPLE_CONF), file=sys.stderr)

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

    if args.pid_file:
        daemon_args['pid_file'] = args.pid_file

    daemonize(**daemon_args)





run()