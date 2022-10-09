from invoke import task


@task
def format(ctx):
    """Run black and isort"""
    for cmd in ("black .", "isort .", "pandoc -s -o README.rst README.md"):
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
def build(ctx):
    """Build"""
    for cmd in (
        "poetry env use 3.8",
        "poetry update",
        "poetry install",
        "poetry run poetry build",
        "poetry env use 3.9",
        "poetry update",
        "poetry install",
        "poetry run poetry build",
        "poetry env use 3.10",
        "poetry update",
        "poetry install",
        "poetry run poetry build",
        "sphinx-apidoc -P -f -o docs/source src/pygeofun",
        "sphinx-build -b html docs dist/docs",
    ):
        ctx.run(cmd, echo=True)
