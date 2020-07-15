from urobots.annopy.project import *
from urobots.annopy.value import *

import json
import os

with open(os.path.join(os.path.dirname(__file__), 'pkginfo.json'), 'r') as f:
    pkginfo = json.load(f)

__version__ = pkginfo["__version__"]

