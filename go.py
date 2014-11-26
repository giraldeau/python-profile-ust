import os
import sys
sys.path.append(os.path.join(os.curdir, "build/lib.linux-x86_64-3.5"))

def cafe():
    print("unicode!")

import lttngProfile
sys.setprofile(lttngProfile.callback)
print("coucou")
cafe()
