from linuxProfile.api import sampling
from nose.tools import assert_equals, assert_raises

def test_import_sampling():
    assert_equals(len(sampling.__dict__), 65)
    print(sampling.TYPE_HARDWARE)