from distutils.core import setup
from Cython.Build import cythonize
from distutils.extension import Extension

ext = [
    Extension('foo1', ['foo1.pyx'],
              extra_compile_args = ['-O2', '-g'],
    ),
    Extension('foo2', ['foo2.pyx'],
              extra_compile_args = ['-O2', '-finstrument-functions', '-g'],
    ),
]

setup(
    ext_modules = cythonize(ext),
)
