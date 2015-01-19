import sys, traceback
from linuxProfile import api

class Foo(object):
    def __init__(self):
        pass
    def foo(self):
        self.bar()
    def bar(self):
        self.baz()
    def baz(self):
        api.ust_traceback()
        exc_type, exc_value, exc_traceback = sys.exc_info()
        traceback.print_tb(exc_traceback, file=sys.stdout)

def test_ust_traceback():
    foo = Foo()
    foo.foo()