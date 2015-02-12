from linuxProfile import api
from linuxProfile.utils import Timespec, clock_gettime_timespec

def test_clock_gettime():
    ts = api.clock_gettime()
    assert(ts[0] > 0 and ts[1] >= 0)

def test_clock_gettime_timespec():
    ts = clock_gettime_timespec()
    assert(ts.tv_sec > 0 and ts.tv_nsec >= 0)

def test_timespec_float():
    ts = Timespec(3, 141600000)
    assert(abs(ts.float() - 3.1416) < 0.0001)

def test_timespec_cmp():
    t1 = Timespec(1, 10)
    t2 = Timespec(1, 20)
    assert(t1 < t2)

    t1 = Timespec(1, 10)
    t2 = Timespec(2, 10)
    assert(t1 < t2)
    
    t1 = Timespec(10, 10)
    t2 = Timespec(10, 10)
    assert(t1 == t2)

def test_timespec_sub():
    t1 = Timespec(10, 2)
    t2 = Timespec(20, 1)
    diff = t2.sub(t1)
    assert(diff == Timespec(9, 999999999))
