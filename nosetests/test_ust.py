import subprocess
import glob
import os
import sys
import tempfile
from nose import with_setup

def run_interpreter(mod, exp, env):
    mod = os.path.join("nosetests", mod)
    cmd = ["python3", "-O", "-m", "linuxProfile.ust", mod]
    with tempfile.TemporaryFile(mode="w+") as out:
        ret = subprocess.call(cmd, env=env, stdout=out, stderr=out)
        if (ret != exp['status']):
            print('Error in test ' + repr(mod))
            print('expected=%d actual=%d' % (exp['status'], ret))
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

def test_cases():
    paths = glob.glob("build/*/linuxProfile")
    if len(paths) != 1:
        raise RuntimeError("rebuild and retry")
    lib = os.path.dirname(paths[0]);
    path = os.path.join(os.path.abspath(os.curdir), lib)
    env = { 'PYTHONPATH': path, 'HOME': os.environ['HOME']}
    for mod, exp in mod_desc.items():
        yield run_interpreter, mod, exp, env
