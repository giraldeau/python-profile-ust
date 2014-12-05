from distutils.core import setup
from Cython.Build import cythonize
from distutils.extension import Extension

ext = [
    Extension('foo1', ['foo1.pyx'],
              extra_compile_args = ['-g'],
    ),
    Extension('foo2', ['foo2.pyx'],
              extra_compile_args = ['-finstrument-functions', '-g'],
    ),
]

ext_trace = [
    Extension('foo3', ['foo3.pyx'],
    ),
]

setup(
    ext_modules = cythonize(ext) +
                  cythonize(ext_trace, compiler_directives={'profile': True}),
)
