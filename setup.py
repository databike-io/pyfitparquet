import os, re, shutil, subprocess, setuptools
from setuptools.command.build_ext import build_ext


# Ignores use_base if $CONDA_DEFAULT_ENV != 'base'
def get_conda_install_dir(use_base=False):
#{
    cdef= os.getenv('CONDA_DEFAULT_ENV')
    cenv = os.getenv('CONDA_PREFIX')
    if not cenv: return None
    elif cdef != 'base': return cenv
    else:
        # Probably mid conda env create, thus infer
        # most recently modified directory is current
        if use_base: return os.getenv('CONDA_PREFIX_1')
        envdir = os.path.join(cenv, 'envs')
        envirs = sorted([os.path.join(envdir, fd) for fd in os.listdir(envdir) 
            if os.path.isdir(os.path.join(envdir, fd))], key=os.path.getmtime)
        return envirs[-1] 
#}

# Relative to prefix in get_conda_install_dir
def find_pyinstall_suffix(conda_prefix, pkg, idir='site-packages'):
    for root, dirs, files in os.walk(conda_prefix):
        if idir in dirs: return os.path.relpath(
            os.path.join(root, idir, pkg), conda_prefix)
    return None


# Class overloads for CMake build support
class CMakeExtension(setuptools.Extension):
    def __init__(self, name):
        super().__init__(name, sources=[])

class BuildExtension(build_ext):
#{
    def run(self):
        for ext in [e for e in self.extensions if isinstance(e, CMakeExtension)]:
            self.build_extension(ext)

    def build_extension(self, ext):
        if self.inplace: print(f"Editable install build not supported. Use build.sh directly.")
        else:
        #{
            conda_prefix = get_conda_install_dir()
            pyinstall_suffix = find_pyinstall_suffix(conda_prefix, ext.name)
            cmake_configure = ['cmake', f'-S{ext.name}/cpp', '-Bpyfit-build',
                               f'-DCMAKE_PREFIX_PATH={conda_prefix}',
                               f'-DCMAKE_INSTALL_PREFIX={conda_prefix}',
                               f'-DINSTALL_SITE_PKGS={pyinstall_suffix}']
            
            # CMake configure, build, and install
            subprocess.check_call(cmake_configure)
            subprocess.check_call(['cmake', '--build', 'pyfit-build', '--target', 'install']) 
        #}
#}

# Setup
package, long_descr = 'pyfitparquet', ''
with open('README.md') as fhd: long_descr = fhd.read()

setuptools.setup(
    name=package,
    description='Transform FIT/TCX files into columnar Parquet format.',
    long_description=long_descr,
    license='GNU General Public License',
    version='0.1.0',
    author='AJ Donich',
    maintainer='AJ Donich',
    author_email='ajdonich@gmail.com',
    url='https://github.com/databike-io/pyfitparquet',
    platforms='Linux/Unix/MacOS',
    packages=[package],
    ext_modules=[CMakeExtension(name=package)],
    cmdclass={'build_ext': BuildExtension},
)
