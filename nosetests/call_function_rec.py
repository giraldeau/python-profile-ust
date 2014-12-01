def rec(n):
    if (n > 0):
        rec(n - 1)

if __name__=='__main__':
    rec(3)