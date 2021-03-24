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


## Examples

```python
from geofun2 import Position, Vector

# Just off Hoek van Holland
org = Position(52.0, 4.0)
nm95 = 95 * 1852.0

# Go west 95 nm to Felixstowe
rmbv = Vector(270.0, nm95)
pos1 = org + rmbv

# Go to the same point using great circle line
gcv = pos1 / org
pos2 = org * gcv

# We should end up at the same location
assert pos1 == pos2

# How disappointing: we managed to gain just 9m by crossing the
# North sea using a great circle :p
assert nm95 - gcv.length == 9.101067085022805, f'Unexpected: {gcv.length}'

print(f'From {org} to {pos1}')
print(f'Rhumb: {rmbv}')
print(f'Great circle: {gcv}')

# Another verification
assert pos1 - org == rmbv
assert pos1 / org == gcv
```

## Classes

Position
 - latitude
 - longitude

Vector
 - azimuth
 - length

Point
 - x
 - y

## Functions

`get_version() -> str`

   Get the library version

`geodesic_direct(latitude: float, longitude: float, azimuth: float, distance: float) -> tuple`

   Get position and final azimuth after moving distance along great circle with starting azimuth

`geodesic_inverse(latitude: float, longitude: float, azimuth: float, distance: float) -> tuple`

   Get starting azimuth, distance and ending azimuth of great circle between positions

`rhumb_direct(latitude: float, longitude: float, azimuth: float, distance: float) -> tuple`

   Get position and final azimuth after moving distance from starting position at fixed azimuth/along rhumb line

`rhumb_inverse(latitude1: float, longitude1: float, latitude2: float, longitude2: float) -> tuple`

   Get rhumb line azimuth, distance and final azimuth between positions

`angle_diff(arg0: numpy.ndarray[numpy.float64], arg1: numpy.ndarray[numpy.float64]) -> object`

   Signed difference between to angles

`angle_mod(arg0: numpy.ndarray[numpy.float64]) -> object`

   Return angle bound to [0.0, 360.0>

`angle_mod_signed(arg0: numpy.ndarray[numpy.float64]) -> object`

   Return angle bound to [-180.0, 180.0>
