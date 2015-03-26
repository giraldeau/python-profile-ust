from linuxProfile.api import sampling
from nose.tools import assert_equals, assert_raises

def modify_events():
    sampling.events['hello'] = 42

def test_import_sampling():
    assert_equals(len(sampling.events.keys()), 60)
    assert_raises(TypeError, modify_events)