
txt = u"""
from lttngProfile import api
def %(func)s():
     return api.is_frame_utf8()
"""

names = {
    'ascii': u'foo',
    'utf8': u'foo_' + unichr(),
    'ucs1': u'',
    'ucs2': u'',
    'ucs4': u'',
}

def test_encoding():
    for k, v in names.items():
        print(repr(v))