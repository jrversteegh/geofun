# GeoFun2

Successor to `geofun` C++ and python library for doing geographic calculations like distance, azimuth and position determination
for geodesics and rhumb lines, orthodromes and loxodromes, respectively.

This version makes use of GeographicLib for doing most of the calculations.

## Building

Currently only building on linux is supported. You'll need `c++`, `make` and `cmake`. When you have those, you should be able
to run:

`
make build && make install
`

You should also be able to add this module to your python project's virtual environment by add the following to your `requirements.txt`:

`
git+https://github.com/jrversteegh/geofun2.git@master#egg=geofun2
`

`pip install -r requirements.txt` should then automatically install geofun2 in your virtual environment.


## Functions

Output of `help(geofun2)`:

    NAME
        geofun2 - Geographic utilities: orthodrome/loxodrome, geodesic/rhumb line evaluation.

    FUNCTIONS
        geodesic_direct(...) method of builtins.PyCapsule instance
            geodesic_direct(latitude: float, longitude: float, azimuth: float, distance: float) -> tuple

            Get position and final azimuth after moving distance along great circle with starting azimuth

        geodesic_inverse(...) method of builtins.PyCapsule instance
            geodesic_inverse(latitude: float, longitude: float, azimuth: float, distance: float) -> tuple

            Get starting azimuth, distance and ending azimuth of great circle between positions

        get_version(...) method of builtins.PyCapsule instance
            get_version() -> str

            Get the library version

        rhumb_direct(...) method of builtins.PyCapsule instance
            rhumb_direct(latitude: float, longitude: float, azimuth: float, distance: float) -> tuple

            Get position and final azimuth after moving distance from starting position at fixed azimuth/along rhumb line

        rhumb_inverse(...) method of builtins.PyCapsule instance
            rhumb_inverse(latitude1: float, longitude1: float, latitude2: float, longitude2: float) -> tuple

            Get rhumb line azimuth, distance and final azimuth between positions
