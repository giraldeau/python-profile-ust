def dummy():
    pass

if __name__=='__main__':
    import lttngProfile
    lttngProfile.set_callback(dummy)
    lttngProfile.do_callback()
    