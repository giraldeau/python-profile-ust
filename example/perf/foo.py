def baz():
    x = []
    i = 0
    while(i < (1 << 20)):
        x.append(i)
        i += 1
        print(i)
    x.sort(reverse=True)
    print(x[:10])

def bar():
    baz()

def foo():
    bar()

if __name__=='__main__':
    foo()

