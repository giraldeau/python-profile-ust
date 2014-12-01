class FooException(Exception):
    pass

def crash():
    raise FooException("doh!") 

if __name__=="__main__":
    crash()
