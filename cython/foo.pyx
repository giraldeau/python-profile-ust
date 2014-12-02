import bidon

def baz():
    bidon.bidon()

def bar():
    baz()

def foo():
    bar()

if __name__=='__main__':
    foo()

