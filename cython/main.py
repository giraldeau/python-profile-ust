import foo1, foo2, foo3

import lttngProfile
lttngProfile.enable()
foo1.foo()
foo2.foo()
foo3.foo()
lttngProfile.disable()
