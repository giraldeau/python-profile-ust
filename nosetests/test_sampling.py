from linuxProfile.api import sampling
from linuxProfile import perf
from nose.tools import assert_equals

def test_import_sampling():
    assert_equals((len(sampling.__dict__) > 60), True)
    assert_equals(sampling.TYPE_SOFTWARE, 1)
    assert_equals(sampling.TYPE_HARDWARE, 0)

def test_create_event():
    type, config = perf.SW_CPU_CLOCK
    print(perf.SW_CPU_CLOCK)
    ev = sampling.Event(type=type, config=config);
    assert_equals(ev.type, type)
    assert_equals(ev.config, config)

def test_methods():
    sampling.open()
    sampling.close()
    sampling.enable()
    sampling.disable()


"""
class Event(object):
    attr
"""
