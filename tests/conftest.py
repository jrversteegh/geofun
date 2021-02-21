import os
import sys
import errno
import logging
import pytest


import __main__ as main

scriptdir = os.path.dirname(os.path.realpath(__file__))
os.chdir(scriptdir)
_module_path = scriptdir + '/..'
sys.path.insert(0, _module_path)

logdir = scriptdir + '/logs'
try:
    os.mkdir(logdir)
except (OSError, FileExistsError) as e:
    if e.errno != errno.EEXIST:
        print(e.errno, errno.EEXIST)
        raise e
except Exception as e:
    print(e)

_logname = f'{logdir}/{os.path.splitext(os.path.basename(main.__file__))[0]}.log'

logging.basicConfig(
    filename=_logname,
    level=logging.DEBUG,
    filemode='w',
    format='%(asctime)s - %(levelname)s - %(name)s - %(message)s'
)
