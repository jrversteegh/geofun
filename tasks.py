import inspect
import os
import platform
from pathlib import Path
from invoke import task

os.chdir(Path(__file__).parent)


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
        for pyver in ("3.9.13", "3.10.11", "3.11.5"):
            cmds += [
                "rmdir /S /Q build && echo 'Removed build dir' || echo 'No previous build'",
                f"pyenv install {pyver}",
                f"pyenv local {pyver}",
                "pyenv local",
                "pyenv exec python --version",
                "pyenv exec poetry env use python",
                "pyenv exec poetry update",
                "pyenv exec poetry run poetry build -vv",
                "pyenv exec poetry run poetry install",
            ]
    else:
        for pyver in ("3.9", "3.10", "3.11", "3.12"):
            cmds += [
                "rm -rf build",
                f"poetry env use {pyver}",
                "poetry update",
                "poetry run poetry build -vv",
                "poetry run poetry install",
            ]

    for cmd in cmds:
        ctx.run(cmd, echo=True)


@task(build)
def publish(ctx):
    """Publish"""
    cmds = [
        "poetry publish",
    ]

    for cmd in cmds:
        ctx.run(cmd, echo=True)
