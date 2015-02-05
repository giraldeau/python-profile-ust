import foo1, foo2, foo3

from linuxProfile.api import enable_ust, disable_ust
enable_ust()
foo1.foo()
foo2.foo()
foo3.foo()
disable_ust()
