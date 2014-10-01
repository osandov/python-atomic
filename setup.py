from distutils.core import setup, Extension

base_module = Extension('atomic', ['atomic_module.c', 'atomic_integer.c'])

setup(
    name='atomic',
    version='0.1',
    description='Module providing types supporting atomic operations',
    author='Omar Sandoval',
    author_email='osandov@osandov.com',
    url='https://github.com/osandov/python-atomic',
    ext_modules=[base_module])
