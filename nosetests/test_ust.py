import subprocess
import glob
import os
import sys
import tempfile
from nose import with_setup

def setup_import():
    sonames = glob.glob("build/*/*.so")
    if len(sonames) != 1:
        raise RuntimeError("rebuild and retry")
    for soname in sonames:
        d = os.path.dirname(soname)
        p = os.path.join(os.curdir, d)
        sys.path.append(p)

def run_interpreter(mod, exp, env):
    mod = os.path.join("nosetests", mod)
    cmd = ["python3", "-O", "-m", "profileUST", mod]
    with tempfile.TemporaryFile(mode="w+") as out:
        ret = subprocess.call(cmd, env=env, stdout=out, stderr=out)
        if (ret != exp['status']):
            print('Error in test ' + repr(mod))
            out.seek(0)
            print(out.read())
    assert ret == exp['status']

mod_desc = { "call_function_foo.py": { 'status': 0 },
             "call_function_rec.py": { 'status': 0 },
             "call_function_c.py": { 'status': 0 },
             "call_method.py": { 'status': 0 },
             "call_lambda.py": { 'status': 0 },
             "call_exception.py": { 'status': 1 },
             "call_exception_c.py": { 'status': 1 },
            }

@with_setup(setup_import)
def test_cases():
    sonames = glob.glob("build/*/*.so")
    if len(sonames) != 1:
        raise RuntimeError("rebuild and retry")
    so = os.path.dirname(sonames[0])
    env = { 'PYTHONPATH': so, 'HOME': os.environ['HOME']}
    for mod, exp in mod_desc.items():
        yield run_interpreter, mod, exp, env
