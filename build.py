import os
import sys
import errno
import platform
import contextlib
from pathlib import Path
import subprocess

import tomli
from pybind11.setup_helpers import Pybind11Extension, build_ext

script_dir = Path(os.path.dirname(os.path.realpath(__file__)))

on_windows = platform.system().startswith('Win')

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
    config_flag = ' --config Release' if on_windows else ''
    pic_flag = '' if on_windows else ' -DCMAKE_CXX_FLAGS=-fPIC'
    with dir_context(build_dir):
        os.system(f"{cmake} -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX={install_prefix} {config}{pic_flag} {source_dir}")
        os.system(f"{cmake} --build .{config_flag}")
        os.system(f"{cmake} --install .")

def write_version_file():
    with dir_context(script_dir):
        try:
            # Create header with version info
            with open("pyproject.toml", "rb") as f:
                project = tomli.load(f)
                version = project["tool"]["poetry"]["version"]
        except OSError:
            version = "0.0.0.dev"

    with open("src/geofun/version.h", "w") as f:
        f.write(f'#define VERSION "{version}"')

# Build contrib packages
install_prefix = Path("../../install")

geographic_source = Path("../../geographiclib")
geographic_build = script_dir / "contrib" / "build" / "geographiclib"

fmt_source = Path("../../fmt")
fmt_build = script_dir / "contrib" / "build" / "fmt"

python_dir = Path(os.path.dirname(sys.executable))
cmake = python_dir / "cmake"

build_and_install(geographic_build, geographic_source, "-DBUILD_SHARED_LIBS=OFF")
build_and_install(fmt_build, fmt_source, "-DFMT_TEST=OFF -DCMAKE_CXX_FLAGS=-fPIC")

write_version_file()

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
