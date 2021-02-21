from glob import glob
import subprocess
from setuptools import setup
from setuptools.dist import Distribution
from pybind11.setup_helpers import Pybind11Extension, build_ext

subprocess.run(('make', 'geographiclib')

dist = Distribution()
dist.parse_config_files()
dist.parse_command_line()
version = dist.get_option_dict('metadata')['version'][1]
assert version != '', "Expected a package version"

with open('src/version.h', 'w') as f:
    f.write(f'#define VERSION "{version}"')

ext_modules = [
    Pybind11Extension(
        "geofun2",
        sorted(glob("src/*.cpp")),
        include_dirs=['contrib/install/include'],
        library_dirs=['contrib/install/lib'],
        libraries=['Geographic'],
    ),
]

setup(
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules
)

