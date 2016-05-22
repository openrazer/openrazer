#!/usr/bin/env python3

import argparse
from razer_daemon.daemon import daemonize

def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('-v', '--verbose', action='store_true', help='Don\'t fork and enable verbose logging')

    parser.add_argument('-F', '--foreground', action='store_true', help='Don\'t fork stay in the foreground')

    parser.add_argument('--config', type=str, help='Location of the config file', default='/etc/razer.conf')
    parser.add_argument('--run-dir', type=str, help='Location of the data directory', default='/var/run/razer')
    parser.add_argument('--log-dir', type=str, help='Location of the log directory')
    parser.add_argument('--pid-file', type=str, help='Location of the pid file')

    return parser.parse_args()

def run():
    args = parse_args()

    daemon_args = {
        'verbose': args.verbose,
        'foreground': args.foreground,
    }
    if 'log_dir' in args:
        daemon_args['log_dir'] = args.log_dir

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