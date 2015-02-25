import traceback
import argparse
import sys
import os

from linuxProfile.commands.report import ReportCommand
from linuxProfile.commands.record import RecordCommand

cmds = {
    'record': RecordCommand,
    'report': ReportCommand,
}

def main():
    parser = argparse.ArgumentParser(description='pyperf')
    parser.add_argument('-v', '--verbose', dest='verbose',
                        action='store_true', default=False,
                        help='verbose mode')
    sub = parser.add_subparsers(help="sub-command help");
    for cmd in cmds.keys():
        p = sub.add_parser(cmd, help="command %s" % (cmd))
        handler = cmds[cmd]()
        handler.arguments(p)
        p.set_defaults(obj=handler)
    args = parser.parse_args()
    if not "obj" in args.__dict__:
        parser.print_usage()
        sys.exit(1)
    try:
        args.obj.handle(args)
    except Exception as e:
        if args.verbose:
            traceback.print_exc()
        print(e)
