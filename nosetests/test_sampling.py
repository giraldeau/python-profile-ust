from linuxProfile.api import sampling
from linuxProfile.launch import ProfileRunnerPerfSampling
from linuxProfile.utils import do_page_faults
from nose.tools import assert_equals
import sys

def test_import_sampling():
    assert_equals((len(sampling.__dict__) > 60), True)
    assert_equals(sampling.TYPE_SOFTWARE, 1)
    assert_equals(sampling.TYPE_HARDWARE, 0)

def test_create_event():
    type, config = ProfileRunnerPerfSampling.evdefs['cycles']
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
    type, config = ProfileRunnerPerfSampling.evdefs['cycles']
    ev = sampling.Event(type=type, config=config)
    sampling.open(ev)
    assert_equals(ev.status, sampling.EVENT_STATUS_OPENED);
    sampling.close()

def test_hits():
    n = 100
    ev = sampling.Event(type=sampling.TYPE_SOFTWARE,
                        config=sampling.COUNT_SW_PAGE_FAULTS,
                        sample_period=1,
                        freq=0)
    sampling.open(ev)
    sampling.enable()
    assert_equals(ev.status, sampling.EVENT_STATUS_OPENED)
    do_page_faults(n)
    act = ev.read()
    sampling.close()
    assert(act >= n)
    assert(sampling.hits() >= n)
    assert_equals(act, sampling.hits(), "read and hits values must be equals")
