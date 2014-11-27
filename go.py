import os
import sys
import glob

sonames = glob.glob("build/*/*.so")
if len(sonames) != 1:
    raise RuntimeError("rebuild and retry")
for soname in sonames:
    d = os.path.dirname(soname)
    p = os.path.join(os.curdir, d)
    sys.path.append(p)

def cafe():
    print("unicode!")

import lttngProfile
sys.setprofile(lttngProfile.callback)
print("coucou")
cafe()
