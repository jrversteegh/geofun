PYTHON := python3
PYTHON_VERSION := $(shell if [ -d venv ]; then . venv/bin/activate; fi; $(PYTHON) -c "import sys; print(f'{sys.version_info[0]}.{sys.version_info[1]}')")
SOURCE := $(wildcard src/*.cpp)
VERSION := $(shell awk -F "=" '/version/ {print $$2}' setup.cfg | sed "s/ //g")
EXTENSION_SUFFIX := $(shell if [ -d venv ]; then . venv/bin/activate; fi; python$(PYTHON_VERSION)-config --extension-suffix)
WHEEL_NAME := $(shell if [ -d venv ]; then . venv/bin/activate; fi; $(PYTHON) -c "from setuptools.dist import Distribution;dist=Distribution(attrs={'name': 'geofun2','version':'$(VERSION)','ext_modules':['geofun2']});cmd=dist.get_command_obj('bdist_wheel');cmd.ensure_finalized();tag='-'.join(cmd.get_tag());print(f'dist/{cmd.wheel_dist_name}-{tag}.whl')")

.PHONY: all
all: $(ALL_TARGETS) test
	
contrib/geographiclib/CMakeLists.txt:
	@git submodule update --init

contrib/geographiclib/BUILD/Makefile: contrib/geographiclib/CMakeLists.txt
	@which cmake >/dev/null || (echo "cmake is required to build" && exit 1)
	@mkdir -p contrib/geographiclib/BUILD
	@cd contrib/geographiclib/BUILD && cmake -DCMAKE_INSTALL_PREFIX=../../install -DGEOGRAPHICLIB_LIB_TYPE=STATIC -DCMAKE_CXX_FLAGS=-fPIC ..

contrib/install/lib/libGeographic.a: contrib/geographiclib/BUILD/Makefile
	@which make >/dev/null || (echo "make is required to build" && exit 1)
	@mkdir -p contrib/install
	@cd contrib/geographiclib/BUILD && make -j 4 install

$(WHEEL_NAME): venv/updated $(SOURCE) contrib/install/lib/libGeographic.a
	@echo "Building: $(WHEEL_NAME) from $(SOURCE)"
	@. venv/bin/activate; $(PYTHON) setup.py bdist_wheel

.PHONY: build
build: contrib/install/lib/libGeographic.a $(WHEEL_NAME)

venv/lib/python$(PYTHON_VERSION)/site-packages/geofun2$(EXTENSION_SUFFIX): $(WHEEL_NAME)
	@echo "Installing: $@"
	@if [ ! -f $(WHEEL_NAME) ]; then echo "Wheel doesn't exist. Do make again"; make; exit 0; else . venv/bin/activate; pip install $(WHEEL_NAME); fi
	@if [ -f "$@" ]; then touch "$@"; fi

.PHONY: install
install: venv/lib/python$(PYTHON_VERSION)/site-packages/geofun2$(EXTENSION_SUFFIX)

.PHONY: test
test: codestyle install
	@echo "Running tests...."
	@. venv/bin/activate; pytest -v tests
	@echo "Done."

test_%: packages
	@. venv/bin/activate; tests/$@.py

.PHONY: codestyle
codestyle: packages
	@. venv/bin/activate; which flake8 >/dev/null || (echo "flake8 checker not available" && exit 1)
	@echo "Checking python code style (PEP8)"
	@. venv/bin/activate; flake8 tests
	@echo "Done."

.PHONY: clean
clean:
	@echo "Cleaning up python cache..."
	@find . -type d -name __pycache__ | xargs rm -rf
	@echo "Cleaning up egg-info..."
	@find . -type d -name "*.egg-info" | xargs rm -rf
	@echo "Cleaning up build..."
	@rm -rf build
	@echo "Cleaning up dist..."
	@rm -rf dist
	@echo "Done."

.PHONY: distclean
distclean: clean
	@echo "Cleaning up contrib install..."
	@rm -rf contrib/install
	@echo "Cleaning up GeographicLib..."
	@rm -rf contrib/geographiclib/BUILD
	@echo "Removing virtual environment..."
	@which $(PYTHON) | grep venv >/dev/null 2>/dev/null && echo "Deactivate your virtual environment first" && exit 1 || echo "Virtual environment not active" 
	@rm -rf venv
	@echo "Done."

.PHONY: packages
packages: venv/updated

venv/bin/activate:
	@echo "Setting up virtual environment..."
	@(which python3.9 >/dev/null && which python3.9-config >/dev/null && python3.9 -mvenv venv) || \
	   (which python3.8 >/dev/null && which python3.8-config >/dev/null && python3.8 -mvenv venv) || \
	   (which python3.7 >/dev/null && which python3.7-config >/dev/null && python3.7 -mvenv venv) || \
	   (echo "No python that has development files and virtualenv installed found. Maybe do 'apt install python3-dev python3-venv'?"; exit 1)
	@echo "Done."

venv/updated: venv/bin/activate requirements.txt
	@echo "Installing packages ..." 
	@. venv/bin/activate \
		&& pip install wheel \
		&& pip install ipython \
		&& pip install -r requirements.txt
	@touch venv/updated
	@echo "Done."
