import subprocess
import glob
import os
import sys
from nose import with_setup

def setup_import():
    sonames = glob.glob("build/*/*.so")
    if len(sonames) != 1:
        raise RuntimeError("rebuild and retry")
    for soname in sonames:
        d = os.path.dirname(soname)
        p = os.path.join(os.curdir, d)
        sys.path.append(p)

def run_interpreter(mod):
    sonames = glob.glob("build/*/*.so")
    if len(sonames) != 1:
        raise RuntimeError("rebuild and retry")
    so = os.path.dirname(sonames[0])
    env = os.environ
    env['PYTHONPATH'] = so
    ret = subprocess.call(["python3", "-O", "-m", "profileUST", mod], env=env)
    print("%s: %d" % (mod, ret))

mod_list = [ "call_function_foo.py",
             "call_function_rec.py",
             "call_function_c.py",
             "call_method.py",
             "call_lambda.py",
             "call_exception.py",
             "call_exception_c.py",
            ]

@with_setup(setup_import)
def test_cases():
    for mod in mod_list:
        yield run_interpreter, os.path.join("nosetests", mod)
