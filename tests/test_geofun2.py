#!/usr/bin/env python3

import os
import pytest
from copy import copy

try:
    import conftest
except ImportError:
    from . import conftest

from geofun2 import *


def test_version():
    assert get_version() == "0.0.2"


def test_angle_mod():
    assert angle_mod(0) == 0
    assert angle_mod(720) == 0
    assert angle_mod(630) == 270
    assert angle_mod(-90) == 270
    assert angle_mod_signed(0) == 0
    assert angle_mod_signed(720) == 0
    assert angle_mod_signed(630) == -90
    assert angle_mod_signed(-90) == -90


def test_rhumb_direct(log):
    lat, lon, azi = rhumb_direct(52., 4., 45., 10000)
    log.debug(f'lat: {lat}, lon: {lon}, azi: {azi}')
    assert lat == pytest.approx(52.0635499025, abs=1E-8)
    assert lon == pytest.approx(4.10303268597, abs=1E-8)
    assert azi == pytest.approx(45., abs=1E-8)


def test_rhumb_inverse(log):
    azi1, dist, azi2 = rhumb_inverse(52., 4., 52.0635499025, 4.10303268597)
    log.debug(f'azi1: {azi1}, dist: {dist}, azi2: {azi2}')
    assert azi1 == pytest.approx(45., abs=1E-8)
    assert azi2 == pytest.approx(45., abs=1E-8)
    assert dist == pytest.approx(10000, abs=1E-4)


def test_geodesic_direct(log):
    lat, lon, azi = geodesic_direct(52., 4., 45., 10000.)
    log.debug(f'lat: {lat}, lon: {lon}, azi: {azi}')
    assert lat == pytest.approx(52.0635048312, abs=1E-8)
    assert lon == pytest.approx(4.10310567353, abs=1E-8)
    assert azi == pytest.approx(45.0812835607, abs=1E-8)


def test_geodesic_inverse(log):
    azi1, dist, azi2 = geodesic_inverse(52., 4., 52.0635048312, 4.10310567353)
    log.debug(f'azi1: {azi1}, dist: {dist}, azi2: {azi2}')
    assert azi1 == pytest.approx(45., abs=1E-8)
    assert azi2 == pytest.approx(45.0812835607, abs=1E-8)
    assert dist == pytest.approx(10000, abs=1E-4)


def test_point():
    p1 = Point(1, 8.88)
    p2 = Point((1, 8.88))
    p3 = Point([1, 8.88])
    assert p2 == p1
    assert p3 == p1
    assert p1.x == pytest.approx(1.0)
    assert p1.y == pytest.approx(8.88)
    p2 = Point(p1)
    p1.x = 2.0
    assert p2.x == pytest.approx(1.0)
    assert p2.y == pytest.approx(8.88)
    p3 = Point(-1, 1.11)
    p4 = p2 + p3
    assert p4.x == pytest.approx(0.0)
    assert p4.y == pytest.approx(9.99)


def test_vector():
    v1 = Vector(0, 1)
    v2 = Vector(90, 1)
    v3 = v1 + v2
    assert v3.azimuth == pytest.approx(45)
    assert v3[0] == v3.azimuth
    assert v3[-2] == v3.azimuth
    assert v3[1] == v3.length
    assert v3[-1] == v3.length
    v3.x == pytest.approx(1)
    v3.y == pytest.approx(1)
    with pytest.raises(IndexError):
        v3[2]
    assert copy(v3) == v3.copy()
    assert v3.norm().length == 1
    assert v3.norm().azimuth == 45
    v4 = Vector(270, -1)
    assert v4.azimuth == pytest.approx(90)
    assert v4.length == pytest.approx(1)
    assert v3.dot(v4) == pytest.approx(1)
    assert v3.dot(v4) == pytest.approx(v3.x * v4.x + v3.y * v4.y)
    assert v3.cross(v4) == pytest.approx(1)
    assert v3.cross(v4) == pytest.approx(v3.x * v4.y - v4.x * v3.y)


def test_position(log):
    # Dubious feature: automatic construction from seconds
    assert Position(1, 1) != Position(1., 1.)
    pos1 = Position(45., 1.)
    assert pos1.latitude == pytest.approx(45)
    assert pos1.longitude == pytest.approx(1)
    assert pos1.latitude == pos1[0]
    assert pos1.longitude == pos1[1]
    pos2 = Position(44 * 3600, 3600)
    assert pos2.latitude == pytest.approx(44)
    assert pos2.longitude == pytest.approx(1)
    v1 = pos2 - pos1
    v2 = pos2 / pos1
    assert v1.azimuth == pytest.approx(180)
    assert v2.azimuth == pytest.approx(180)
    assert v1.length == pytest.approx(60 * 1852, abs=3.)
    assert v2.length == pytest.approx(60 * 1852, abs=3.)
    pos1 += v1
    assert pos1 == pos2
    v3 = -v2 * 0.1
    for i in range(10):
        pos2 *= v3
    pos1 = Position(45., 1.)
    assert pos1 == pos2

    pos2 = Position(latitude=45, longitude=1)
    assert pos1 == pos2

    pos2 = pos1 + Vector(90, 40E3)
    assert pos2[1] == pytest.approx(1.507312689879)
    pos2[1] = 358
    assert pos2.longitude == -2

    v = Vector(45, 100000)
    assert pos1 / v == pos1 * -v


def test_string_construction(log):
    p = Position('52.1', '4.1')
    assert p.latitude == pytest.approx(52.1)
    assert p.longitude == pytest.approx(4.1)
    p = Position('7200', '3600')
    assert p.latitude == pytest.approx(2)
    assert p.longitude == pytest.approx(1)
    p = Position('7200 3600')
    assert p.latitude == pytest.approx(2)
    assert p.longitude == pytest.approx(1)
    p = Position('4.1W 12.0N')
    assert p.latitude == pytest.approx(12)
    assert p.longitude == pytest.approx(-4.1)
    p = Position('00°30\'00"S 00°30\'00W"')
    assert p.latitude == pytest.approx(-0.5)
    assert p.longitude == pytest.approx(-0.5)
    p = Position('-00°00\'30" 00°00\'30"')
    assert p.latitude == pytest.approx(-1. / 120.)
    assert p.longitude == pytest.approx(1. / 120.)
    p = Position('-89°30.5 00°00.50')
    assert p.latitude == pytest.approx(-89 - 61. / 120.)
    assert p.longitude == pytest.approx(1. / 120.)


def test_repr_and_str(log):
    p = Point(3.131313, 5.151515)
    assert str(p) == '3.131, 5.152'
    assert repr(p) == 'Point(3.131313, 5.151515)'
    p = Point(300000.131313, 500000.151515)
    assert str(p) == '300000.131, 500000.152'
    assert repr(p) == 'Point(300000.131313, 500000.151515)'
    p = Point(3, 5)
    assert str(p) == '3.000, 5.000'
    assert repr(p) == 'Point(3.0, 5.0)'
    pos = Position('-89°30.5S 00°00.50')
    assert str(pos) == '89.50833333, 0.00833333'
    assert repr(pos) == 'Position(89.5083333333333, 0.00833333333333333)'
    v = Vector(45, 88)
    assert str(v) == '45.000, 88.000'
    assert repr(v) == 'Vector(45.0, 88.0)'
