import foo1, foo2

import lttngProfile
lttngProfile.enable()
foo1.foo()
foo2.foo()
lttngProfile.disable()
