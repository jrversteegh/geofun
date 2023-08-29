import platform
import inspect
# Monkey patch for invoke which isn't py311 ready. See https://github.com/pyinvoke/invoke/issues/833
# TODO: remove once invoke supports python3.11
if not hasattr(inspect, "getargspec"):
    inspect.getargspec = inspect.getfullargspec
from invoke import task


@task
def format(ctx):
    """Run black and isort"""
    for cmd in ("black .", "isort ."):
        ctx.run(cmd, echo=True)


@task
def check(ctx):
    """Run flake8"""
    for cmd in ("flake8 .",):
        ctx.run(cmd, echo=True)


@task
def test(ctx):
    """Run tests"""
    for cmd in ("pytest -vv --cov --junitxml=../build/reports/tests.xml",):
        ctx.run(cmd, echo=True)


@task
def build_doc(ctx):
    for cmd in (
        "sphinx-apidoc -P -f -o docs/source src/pygeofun",
        "sphinx-build -b html docs dist/docs",
    ):
        ctx.run(cmd, echo=True)

@task
def build(ctx):
    """Build"""
    cmds = []

    if platform.platform().startswith("Win"):
        for pyver in ("3.8.10", "3.9.13", "3.10.11", "3.11.5"):
            cmds += [
                f"pyenv install {pyver}",
                f"pyenv local {pyver}",
                "pyenv local",
                "pyenv exec python --version",
                "pyenv exec poetry env use python",
                "pyenv exec poetry update",
                "pyenv exec poetry install",
                "pyenv exec poetry run poetry build",
            ]
    else:
        for pyver in ("3.8", "3.9", "3.10", "3.11"):
            cmds += [
                f"poetry env use {pyver}",
                "poetry update",
                "poetry install",
                "poetry run poetry build",
            ]

    for cmd in cmds:
        ctx.run(cmd, echo=True)

@task
def publish(ctx):
    """Publish"""
    cmds = []

    if platform.platform().startswith("Win"):
        for pyver in ("3.8.10", "3.9.13", "3.10.11", "3.11.5"):
            cmds += [
                f"pyenv install {pyver}",
                f"pyenv local {pyver}",
                "pyenv local",
                "pyenv exec python --version",
                "pyenv exec poetry env use python",
                "pyenv exec poetry update",
                "pyenv exec poetry install",
                "pyenv exec poetry run poetry build",
                "pyenv exec poetry run poetry publish",
            ]
    else:
        for pyver in ("3.8", "3.9", "3.10", "3.11"):
            cmds += [
                f"poetry env use {pyver}",
                "poetry update",
                "poetry install",
                "poetry run poetry build",
                "poetry run poetry publish",
            ]

    for cmd in cmds:
        ctx.run(cmd, echo=True)
