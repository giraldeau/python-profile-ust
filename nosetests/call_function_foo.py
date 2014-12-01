def baz():
    pass

def bar():
    baz()

def foo():
    bar()

if __name__=='__main__':
    foo()