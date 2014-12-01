from distutils.core import setup, Extension
 
module1 = Extension(
    'lttngProfile',
    sources = ['lttngProfile.c'],
    include_dirs=['.'],
    libraries=['lttng-ust', 'dl'],
    extra_compile_args = ['-O2'],
)
 
setup (name = 'PythonProfileUst',
        version = '1.0',
        description = 'Trace Python functions with LTTng-UST',
        ext_modules = [module1],
        py_modules = ['profileUST'],
)

