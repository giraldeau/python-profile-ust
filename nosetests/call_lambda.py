def inc(n):
    return lambda x: x + n

if __name__=="__main__":
    f = inc(42)
    f(0)