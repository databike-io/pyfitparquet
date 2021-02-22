import sys, os, site, subprocess, setuptools
from setuptools.command.install import install
from setuptools._distutils import dir_util

class CondaInstall(install):
#{
    def run(self):
        print(f"CondaInstall - part I")
        conda_prefix = os.environ['CONDA_PREFIX']
        if 'CONDA_BUILD_STATE' in os.environ:
            conda_prefix = os.environ['PREFIX']
        self.run_cmake_build(conda_prefix)

        print(f"CondaInstall - part II")
        install.run(self)
        
        print(f"CondaInstall - part III")
        bdist_dumb = self.get_bdist_prefix(conda_prefix)
        if bdist_dumb: dir_util.copy_tree('cmake-build/install', bdist_dumb)
        else: dir_util.copy_tree('cmake-build/install', conda_prefix)

    def run_cmake_build(self, conda_prefix):
        spkgs_suffix = self.get_site_pkgs_suffix(conda_prefix)        
        cmake_configure = ['cmake', '-Spyfitparquet/cpp', '-Bcmake-build',
                           f'-DCMAKE_PREFIX_PATH={conda_prefix}',
                           f'-DCMAKE_INSTALL_PREFIX=cmake-build/install',
                           f'-DINSTALL_SITE_PKGS={spkgs_suffix}']

        # CMake configure, build, and install
        subprocess.check_call(cmake_configure)
        subprocess.check_call(['cmake', '--build', 'cmake-build', '--target', 'install'])

    def get_site_pkgs_suffix(self, prefix):
        for spkg in site.getsitepackages():
            if spkg.startswith(prefix):
                return os.path.join(os.path.relpath(spkg, prefix), 'pyfitparquet')
        
        pyversion = f'python{sys.version_info[0]}.{sys.version_info[1]}'
        return os.path.join('lib', pyversion, 'site-packages', 'pyfitparquet')

    def get_bdist_prefix(self, prefix):
        for root, dirs, files in os.walk('build'):
            if root.endswith(prefix):
                return root
#}


# Setup
package, long_descr = 'pyfitparquet', ''
with open('README.md') as fhd: long_descr = fhd.read()

setuptools.setup(
    name=package,
    description='ETL support of Garmin FIT/TCX files to Apache Parquet',
    long_description=long_descr,
    version='1.0',
    license='Apache License 2.0',
    url='https://github.com/databike-io/pyfitparquet',
    author='AJ Donich',
    author_email='ajdonich@gmail.com',
    keywords='parquet garmin fit tcx',
    platforms='Linux/Unix/MacOS',
    packages=[package],
    package_data = {package: ["*.yml", "*.so"]},
    cmdclass={'install': CondaInstall}
)
