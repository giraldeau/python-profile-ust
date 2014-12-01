import sys
import lttngProfile

'''
Launcher for LTTng-UST
'''

class ProfileUST(object):
    def runctx(self, cmd, globals):
        lttngProfile.enable()
        try:
            exec(cmd, globals)
        finally:
            lttngProfile.disable()
        return self

def main():
    import os
    import sys
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('remainder', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    
    if len(args.remainder) > 0:
        progname = args.remainder[0]
        sys.path.insert(0, os.path.dirname(progname))
        with open(progname, 'rb') as fp:
            code = compile(fp.read(), progname, 'exec')
        globs = {
            '__file__': progname,
            '__name__': '__main__',
            '__package__': None,
            '__cached__': None,
        }
        profile = ProfileUST()
        profile.runctx(code, globs)
    else:
        parser.print_usage()
    return parser

if __name__=='__main__':
    main()
