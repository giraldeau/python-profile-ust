from linuxProfile.api import sampling
from nose.tools import assert_equals

def test_import_sampling():
    print(sampling.events)
    assert_equals(len(sampling.events.keys()), 60)