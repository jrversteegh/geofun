from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

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
