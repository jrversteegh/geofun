import errno
import logging
import os
import sys

import __main__ as main
import pytest

scriptdir = os.path.dirname(os.path.realpath(__file__))
os.chdir(scriptdir)

logdir = scriptdir + "/logs"
try:
    os.mkdir(logdir)
except (OSError, FileExistsError) as e:
    if e.errno != errno.EEXIST:
        print(e.errno, errno.EEXIST)
        raise e
except Exception as e:
    print(e)

_logname = f"{logdir}/{os.path.splitext(os.path.basename(main.__file__))[0]}.log"

logging.basicConfig(
    filename=_logname,
    level=logging.DEBUG,
    filemode="w",
    format="%(asctime)s - %(levelname)s - %(name)s - %(message)s",
)


@pytest.fixture
def log(request):
    result = logging.getLogger(f"test.{request.node.name}")
    yield result
    for handler in result.handlers:
        handler.flush()
