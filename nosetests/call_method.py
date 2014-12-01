class Foo(object):
    def __init__(self):
        pass
    def foo(self):
        self.bar()
    def bar(self):
        self.baz()
    def baz(self):
        pass

if __name__=='__main__':
    foo = Foo()
    foo.foo()