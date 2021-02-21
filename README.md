# GeoFun2

Successor to `geofun` C++ and python library for doing geographic calculations like distance and azimuth and position determination
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
