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
        api.unwind()
        api.traceback()
        api.traceback_full()

def test_traceback_ust():
    foo = Foo()
    foo.foo()
