from linuxProfile import api
from functools import total_ordering
import resource
import mmap
import math
import sys

@total_ordering
class Timespec(object):
    NSEC_PER_SEC = 1000000000
    def __init__(self, tv_sec=0, tv_nsec=0):
        self.tv_sec = tv_sec
        self.tv_nsec = tv_nsec
    def sub(self, other):
        tv_sec = self.tv_sec - other.tv_sec;
        tv_nsec = self.tv_nsec - other.tv_nsec;
        if (self.tv_nsec < other.tv_nsec):
            tv_sec -= 1
            tv_nsec += Timespec.NSEC_PER_SEC
        return Timespec(tv_sec, tv_nsec)
    def float(self):
        return self.tv_sec + float(self.tv_nsec) / Timespec.NSEC_PER_SEC
    def __eq__(self, other):
        return ((self.tv_sec, self.tv_nsec) ==
                (other.tv_sec, other.tv_nsec))
    def __lt__(self, other):
        return ((self.tv_sec, self.tv_nsec) <
                (other.tv_sec, other.tv_nsec))
    def __hash__(self):
        return self.tv_sec.__hash__() + self.tv_nsec.__hash__()
    def __repr__(self):
        return "(%d.%d)" % (self.tv_sec, self.tv_nsec)

def clock_gettime_timespec():
    return Timespec(*api.clock_gettime())

def do_page_faults(n=0):
    pg = resource.getpagesize()
    with mmap.mmap(-1, n * pg) as mm:
        for i in range(n):
            mm.seek(i * pg);
            mm.write(b'x')

class ProgressBar(object):
    def __init__(self, total_work=1, width=40):
        self._total_work = total_work
        self._width = width
        self._state = 0

    @property
    def total_work(self):
        return self._total_work

    @total_work.setter
    def total_work(self, work):
        self._total_work = float(work)

    def update(self, current):
        bar = int(math.ceil((current / self._total_work) * self._width))
        if (bar == self._state):
            return
        space = self._width - bar
        sys.stdout.write("[{}{}]\r".format("#" * bar, " " * space))
        sys.stdout.flush()
        self._state = bar

    def done(self):
        sys.stdout.write("\ndone\n")
        sys.stdout.flush()

class NullProgressBar(ProgressBar):
    def __init__(self, total_work=1, width=40):
        super().__init__(total_work, width)

    def update(self, current):
        pass

    def done(self):
        pass
