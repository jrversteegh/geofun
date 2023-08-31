#!/bin/sh

cd /tmp

for ver in 3.8 3.9 3.10 3.11; do
  python$ver -m venv venv
  . venv/bin/activate
  pip install --only-binary pygeofun pygeofun || echo "########### Failed to get pygeofun"
  python --version
  echo "import geofun; print(f'GEOFUN VERSION: {geofun.get_version()}')" | python
  deactivate
  rm -rf venv
done

