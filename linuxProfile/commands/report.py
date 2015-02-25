import argparse
import sys
from linuxProfile.commands.base import BaseCommand
from linuxProfile.tools import load_trace, build_profile, CalltreeReport, ProfileTree

class ReportCommand(BaseCommand):
    def __init__(self):
        super(BaseCommand, self).__init__()

    def arguments(self, parser):
        parser.add_argument('path', nargs='+', help='trace path')

    def handle(self, args):
        trace = load_trace(args.path)

        root = ProfileTree("root")
        build_profile(trace, root)

        report = CalltreeReport()
        report.write(sys.stdout, root)
