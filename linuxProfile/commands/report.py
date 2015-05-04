import argparse
import sys
import os
import yaml
from linuxProfile.commands.base import BaseCommand
from linuxProfile.tools import load_trace, build_profile, CalltreeReport, ProfileTree, StatItem, profile_rms_error
from linuxProfile.utils import ProgressBar

class ReportCommand(BaseCommand):
    def __init__(self):
        super(BaseCommand, self).__init__()

    def arguments(self, parser):
        parser.add_argument('--profile', default=None, help='profile to compute the RMS error')
        parser.add_argument('--quiet', default=False, action="store_true", help='quiet mode')
        parser.add_argument('path', nargs='+', help='trace path')

    def handle(self, args):
        trace = load_trace(args.path)

        bar = None
        if not args.quiet:
            bar = ProgressBar()
        root = ProfileTree("root")
        ret = build_profile(trace, root, bar)
        print("processed %d events" % (ret.count))

        report = CalltreeReport()
        report.write(sys.stdout, root)
        if (args.profile and os.path.isfile(args.profile)):
            data = None
            try:
                with open(args.profile, "r") as f:
                    data = yaml.load(f)
            except Exception as e:
                 print("Error loading YAML profile {}".format(e))
            if data:
                prefix = data.get("prefix", "")
                profile = data.get("profile", [])
                stats = []
                for k, v in profile.items():
                    path = prefix + "/" + k
                    stats.append(StatItem(path, v, 0, 0))
                rms = profile_rms_error(stats, root)
                print("rms error: %f" % (rms * 100))

