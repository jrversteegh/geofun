#!/usr/bin/env python3

import os
import pytest

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


def test_position(log):
    pos1 = Position(45., 1.)
    assert pos1.latitude == pytest.approx(45)
    assert pos1.longitude == pytest.approx(1)
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
    # No longer equal but off by less than 2mm
    assert pos1 != pos2
    diff = pos1 - pos2
    assert diff.length < 0.002
    v = Vector(45, 100000)
    assert pos1 / v == pos1 * -v
