from setuptools import setup, Extension
import glob
import sys

v = sys.version_info
if v.major < 3:
    sys.stderr.write('Python 3 is required\n')
    sys.exit(1)

ext = [
    Extension(
            'linuxProfile.api',
            sources = glob.glob('ext/*.c'),
            include_dirs=['ext/'],
            libraries=['perfuser', 'lttng-ust', 'dl'],
            #extra_compile_args = ['-O0', '-g'], # FIXME: add --enable-debug option
            extra_compile_args = ['-O2'],
    ),
]

entry_points = {
    'console_scripts': [
        'pyperf = linuxProfile.cli:main'
    ],
}
 
setup (name = 'python-linux-profile',
        version = '1.0',
        description = 'Trace Python under Linux',
        ext_modules = ext,
        packages = ['linuxProfile'],
        entry_points = entry_points,
)

