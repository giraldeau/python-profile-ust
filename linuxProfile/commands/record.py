import argparse
from linuxProfile.commands.base import BaseCommand
from linuxProfile.launch import run_command, ProfileRunnerPerf

class RecordCommand(BaseCommand):
    def __init__(self):
        super(BaseCommand, self).__init__()

    def arguments(self, parser):
        parser.add_argument('script', nargs=argparse.REMAINDER, help='python script')

    def handle(self, args):
        run_command(ProfileRunnerPerf(), args.script)