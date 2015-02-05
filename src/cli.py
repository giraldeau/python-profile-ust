import sys
from linuxProfile import api

'''
Launcher for LTTng-UST
'''

class ProfileRunner(object):
    def runctx(self, cmd, globals):
        self.enable()
        try:
            exec(cmd, globals)
        finally:
            self.disable()
        return self
    def enable(self):
        pass
    def disable(self):
        pass

class ProfileRunnerUST(ProfileRunner):
    def enable(self):
        api.enable_ust()
    def disable(self):
        api.disable_ust()

class ProfileRunnerPerf(ProfileRunner):
    def enable(self):
        api.enable_perf()
    def disable(self):
        api.disable_perf()

def run(prof):
    import os
    import sys
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('remainder', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    
    if len(args.remainder) > 0:
        progname = args.remainder[0]
        sys.path.insert(0, os.path.dirname(progname))
        with open(progname, 'rb') as fp:
            code = compile(fp.read(), progname, 'exec')
        globs = {
            '__file__': progname,
            '__name__': '__main__',
            '__package__': None,
            '__cached__': None,
        }
        prof.runctx(code, globs)
    else:
        parser.print_usage()
    return parser
