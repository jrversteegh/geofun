# GeoFun

Library for doing geographic calculations like distance, azimuth and position determination
for geodesics and rhumb lines, orthodromes and loxodromes, respectively.

This version makes use of GeographicLib for doing most of the calculations.

This is a C++ package that uses pybind11 to wrap the C++ version of GeographicLib, which
makes it faster (~100x) than the pure python version of [geographiclib](https://geographiclib.sourceforge.io/html/python/index.html).

Compare:
```Python
In [1]: from geofun import geodesic_inverse

In [2]: %timeit geodesic_inverse(52, 4, 28, -16.6)
1.17 µs ± 37 ns per loop (mean ± std. dev. of 7 runs, 1000000 loops each)

In [3]: from geographiclib.geodesic import Geodesic

In [4]: %timeit Geodesic.WGS84.Inverse(52, 4, 28, -16.6)
107 µs ± 170 ns per loop (mean ± std. dev. of 7 runs, 10000 loops each)

In [5]: geodesic_inverse(52, 4, 28, -16.6)
Out[5]: (-139.28471885516532, 3168557.154495447, -152.90624110350674)

In [6]: Geodesic.WGS84.Inverse(52, 4, 28, -16.6)
Out[6]:
{'lat1': 52,
 'lon1': 4.0,
 'lat2': 28,
 'lon2': -16.6,
 'a12': 28.519118381735783,
 's12': 3168557.1544954455,
 'azi1': -139.28471885516532,
 'azi2': -152.90624110350674}
```

## Building

-   Get
    [poetry](https://python-poetry.org/docs/master/#installing-with-the-official-installer)
    if you don\'t have it

-   Check out the source code:
    `git clone https://github.com/jrversteegh/geofun.git --recurse-submodules`

-   Execute `poetry build` to build the package or `poetry install` to get a virtual environment 
    to work in. Both require a working modern C++ compiler. GCC 9.4 and MSVC 14.3 were tested. 
    Others may work.


## Examples

Some operator abuse was used to mark the difference between geodesic and mercator based operations.
`+` and `-` are addition and subtraction in the mercator projection (loxodromes) and `*` and `/`
are addition and subtraction on geodesics (orthodromes). If you object to this, you're probably
right. Any suggestions for a better way are quite welcome.

```python
from geofun import Position, Vector

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

`geodesic_inverse(latitude1: float, longitude1: float, latitude2: float, longitude2: float) -> tuple`

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
