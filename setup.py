from glob import glob
from setuptools import setup
from setuptools.dist import Distribution
from pybind11.setup_helpers import Pybind11Extension, build_ext

dist = Distribution()
dist.parse_config_files()
dist.parse_command_line()

with open('src/version.h', 'w') as f:
    f.write(f'#define VERSION {dist.get_option_dict("version")}')

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

