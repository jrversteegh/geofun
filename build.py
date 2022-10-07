import os
import sys
import errno
import contextlib
from pathlib import Path
import subprocess

import tomli
from pybind11.setup_helpers import Pybind11Extension, build_ext

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))


@contextlib.contextmanager
def dir_context(new_dir):
    previous_dir = os.getcwd()
    try:
        os.makedirs(new_dir)
    except (FileExistsError):
        ...
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(previous_dir)

def build_and_install(build_dir, source_dir, config):
    with dir_context(build_dir):
        os.system(f"{cmake} -DCMAKE_INSTALL_PREFIX={install_prefix} {config} {source_dir}")
        os.system(f"{cmake} --build . --parallel 4")
        os.system(f"{cmake} --install .")

# Build contrib packages
install_prefix = Path("../../install")

geographic_source = Path("../../geographiclib")
geographic_build = script_dir / "contrib" / "build" / "geographiclib"

fmt_source = Path("../../fmt")
fmt_build = script_dir / "contrib" / "build" / "fmt"

python_dir = Path(os.path.dirname(sys.executable))
cmake = python_dir / "cmake"

build_and_install(geographic_build, geographic_source, "-DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS=-fPIC")
build_and_install(fmt_build, fmt_source, "-DFMT_TEST=OFF -DCMAKE_CXX_FLAGS=-fPIC")

# Create header with version info
with open("pyproject.toml", "rb") as f:
    project = tomli.load(f)
    version = project["tool"]["poetry"]["version"]

with open("src/geofun/version.h", "w") as f:
    f.write(f'#define VERSION "{version}"')


def build(setup_kwargs):
    ext_modules = [
        Pybind11Extension(
            "geofun",
            sources=[str(f) for f in sorted(script_dir.glob("src/geofun/*.cpp"))],
            include_dirs=["contrib/install/include"],
            library_dirs=["contrib/install/lib"],
            libraries=["GeographicLib", "fmt"],
        ),
    ]
    setup_kwargs.update(
        {
            "ext_modules": ext_modules,
            "cmdclass": {"build_ext": build_ext},
            "zip_safe": False,
        }
    )
