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
        print("sys_traceback depth=%d" % (len(traceback.extract_stack())))
        api.ust_traceback()

def test_ust_traceback():
    foo = Foo()
    foo.foo()