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


def test_rhumb_direct():
    lat, lon = rhumb_direct(52, 4, 45, 10000)
    assert lat == pytest.approx(52.06355)
    assert lon == pytest.approx(4.1030327)
