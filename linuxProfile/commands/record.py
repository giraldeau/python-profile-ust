import argparse
from linuxProfile.commands.base import BaseCommand
from linuxProfile.launch import run_command, ProfileRunnerPerfSampling

class RecordCommand(BaseCommand):
    def __init__(self):
        super(BaseCommand, self).__init__()

    def arguments(self, parser):
        parser.add_argument('--event', '-e', default='cycles', choices=ProfileRunnerPerfSampling.evdefs.keys(), help='event to enable')
        parser.add_argument('--period', '-p', default=10000, help='sampling period')
        parser.add_argument('script', nargs=argparse.REMAINDER, help='python script')

    def handle(self, args):
        if (len(args.script) == 0):
            return
        if (args.script[0] == '--'):
            args.script = args.script[1:]
        print(args)
        run_command(ProfileRunnerPerfSampling(args.event, args.period), args.script)