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
        api.traceback_ust()

def test_traceback_ust():
    foo = Foo()
    foo.foo()
