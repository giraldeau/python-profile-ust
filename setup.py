from distutils.core import setup, Extension
 
module1 = Extension(
    'lttngProfile',
    sources = ['lttngProfile.c', 'tp.c'],
    include_dirs=['.'],
    libraries=['lttng-ust'],
)
 
setup (name = 'PythonProfileUst',
        version = '1.0',
        description = 'Trace Python functions with LTTng-UST',
        ext_modules = [module1])

