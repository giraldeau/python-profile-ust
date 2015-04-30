import argparse
import sys
import os

from linuxProfile.api import enable_perf, disable_perf, enable_ust, disable_ust, sampling

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
        enable_ust()
    def disable(self):
        disable_ust()

class ProfileRunnerPerf(ProfileRunner):
    def enable(self):
        enable_perf()
    def disable(self):
        disable_perf()

class ProfileRunnerPerfSampling(ProfileRunner):
    evdefs = {
        "cycles":           (sampling.TYPE_HARDWARE, sampling.COUNT_HW_CPU_CYCLES),
        "instructions":     (sampling.TYPE_HARDWARE, sampling.COUNT_HW_INSTRUCTIONS),
        "cache-references": (sampling.TYPE_HARDWARE, sampling.COUNT_HW_CACHE_REFERENCES),
        "cache-misses":     (sampling.TYPE_HARDWARE, sampling.COUNT_HW_CACHE_MISSES),
    }
    def __init__(self, event, period):
        if not event in self.evdefs.keys():
            raise RuntimeError("unkown event")
        self._event = event
        self._period = period
    def enable(self):
        (ev_type, ev_config) = self.evdefs[self._event]
        enable_perf()
        ev = sampling.Event(type=ev_type,
                            config=ev_config,
                            sample_period=self._period,
                            freq=0)
        sampling.open(ev)
        sampling.enable()
    def disable(self):
        sampling.disable()
        disable_perf()

def run(prof):
    parser = argparse.ArgumentParser()
    parser.add_argument('remainder', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    try:
        run_command(prof, args.remainder)
    except Exception as e:
        parser.print_usage()
        sys.exit(1)

def run_command(prof, com):
    if len(com) > 0:
        sys.argv = com
        progname = com[0]
        # find in sys.path for the progname
        if not os.path.isfile(progname):
            for p in sys.path:
                prog = os.path.join(p, progname)
                if (os.path.isfile(prog)):
                    progname = prog
                    break
        print("running %s" % (progname))
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
        raise Exception("invalid command")
