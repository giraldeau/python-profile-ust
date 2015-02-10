from linuxProfile import api
from functools import total_ordering

@total_ordering
class Timespec(object):
    NSEC_PER_SEC = 1000000000
    def __init__(self, tv_sec = 0, tv_nsec = 0):
        self.tv_sec = tv_sec
        self.tv_nsec = tv_nsec
    def sub(self, other):
        tv_sec  = self.tv_sec  - other.tv_sec;
        tv_nsec = self.tv_nsec - other.tv_nsec;
        if (self.tv_nsec < other.tv_nsec):
            tv_sec -= 1
            tv_nsec += Timespec.NSEC_PER_SEC
        return Timespec(tv_sec, tv_nsec)
    def float(self):
        return self.tv_sec + self.tv_nsec / Timespec.NSEC_PER_SEC
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