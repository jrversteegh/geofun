#!/usr/bin/env python3

import os
import pytest

try:
    import conftest
except ImportError:
    from . import conftest

from geofun2 import *


def test_version():
    assert get_version() == "0.0.1"


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
