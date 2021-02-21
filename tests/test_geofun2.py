#!/usr/bin/env python3

import os
import pytest

try:
    import conftest
except ImportError:
    from . import conftest

from geofun2 import *
