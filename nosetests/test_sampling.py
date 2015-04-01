from linuxProfile.api import sampling
from linuxProfile import perf
from nose.tools import assert_equals
import sys

def test_import_sampling():
    assert_equals((len(sampling.__dict__) > 60), True)
    assert_equals(sampling.TYPE_SOFTWARE, 1)
    assert_equals(sampling.TYPE_HARDWARE, 0)

def test_create_event():
    type, config = perf.SW_CPU_CLOCK
    ev = sampling.Event(type=type, config=config);
    assert_equals(ev.type, type)
    assert_equals(ev.config, config)
    assert_equals(ev.status, sampling.EVENT_STATUS_CLOSED)

def test_methods():
    sampling.open()
    sampling.close()
    sampling.enable()
    sampling.disable()

def test_open():
    type, config = perf.SW_CPU_CLOCK
    ev = sampling.Event(type=type, config=config)
    sampling.open(ev)
    assert_equals(ev.status, sampling.EVENT_STATUS_OPENED);
    sampling.close()

def test_hits():
    ev = sampling.Event(type=sampling.TYPE_SOFTWARE,
                        config=sampling.COUNT_SW_PAGE_FAULTS,
                        sample_period = 1)
    sampling.open(ev)
    sampling.enable()
    assert_equals(ev.status, sampling.EVENT_STATUS_OPENED)
    x = [x for x in range(100)]
    assert(ev.read() > 0)
    assert(sampling.hits() > 0)
    assert_equals(ev.read(), sampling.hits(), "hits and read value should be equals")

"""
class Event(object):
    attr
"""
