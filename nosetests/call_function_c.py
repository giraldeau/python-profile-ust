def dummy():
    pass

if __name__=='__main__':
    from linuxProfile import api
    api.set_callback(dummy)
    api.do_callback()
    