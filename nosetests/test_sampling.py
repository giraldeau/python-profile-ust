from linuxProfile.api import sampling
from nose.tools import assert_equals

def test_import_sampling():
    assert_equals((len(sampling.__dict__) > 60), True)
    print(sampling.TYPE_HARDWARE)

def test_create_event():
    ev = sampling.Event()
    print(ev.__doc__)

"""
class Session(object):
    group_fd
    events = []
    def enable(self):
        pass
    def disable(self):
        pass

class Event(object):
    type
    config
    group_fd
    attr
"""