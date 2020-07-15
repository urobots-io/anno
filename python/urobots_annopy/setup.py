# Copyright (C) Urobots GmbH 2017. All Rights Reserved.
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.
# ==============================================================================
"""
Package setup. This package will be distributed as open source, so avoid dependency on other urobot libraries.
"""

import glob
import json
import os
import re
from setuptools import setup

MY_DIR = os.path.dirname(os.path.abspath(__file__))
NS_PATH = os.path.join(MY_DIR, 'urobots', 'annopy')

def files_to_modules(files):
    """
    Convert a path "dir/subdir/file.py" to the list of import-like modules "dir.subdir.file".
    :param files: a list of paths.
    :return: a list of module names.
    """
    modules = [re.sub(".py" + "$", '', x) for x in files]
    modules = [x.replace(os.sep, '.') for x in modules]
    return modules if type(files) == list else modules[0]


with open(os.path.join(NS_PATH, 'pkginfo.json'), 'r') as f:
    pkginfo = json.load(f)

py_files = glob.glob(MY_DIR + "/urobots/**/*.py", recursive=True)
py_files = [f for f in py_files if 'test' not in f]
py_files = [os.path.relpath(os.path.normpath(f), start=MY_DIR) for f in py_files]
py_modules = files_to_modules(py_files)

data_patterns = [
    NS_PATH + '/pkginfo.json',
]

data_files = []
for dp in data_patterns:
    data_files += glob.glob(dp, recursive=True)

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="urobots-annopy",
    version=pkginfo["__version__"],
    author="Urobots GmbH",
    author_email="info@urobots.io",
    description=pkginfo["description"],
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://urobots.io",
    packages=["urobots"],  # Specify only the native namespace to be able to use package_data.
    py_modules=py_modules,
    package_data={
        'urobots': data_files
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Commercial License",
        "Operating System :: OS Independent",
    ],
    install_requires=["urobots-utils", "numpy", "opencv-contrib-python"],
    ext_modules=None
)