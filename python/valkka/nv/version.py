from valkka.core import VERSION_MAJOR as VALKKA_VERSION_MAJOR
from valkka.core import VERSION_MINOR as VALKKA_VERSION_MINOR
from valkka.core import VERSION_PATCH as VALKKA_VERSION_PATCH

# the following three lines are modded by setver.bash for valkka.nv
VERSION_MAJOR=1
VERSION_MINOR=0
VERSION_PATCH=0

version_tag = "%i.%i.%i" % (VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)

# required libValkka version for this extension module to work
MIN_VALKKA_VERSION_MAJOR = 1
MIN_VALKKA_VERSION_MINOR = 2
MIN_VALKKA_VERSION_PATCH = 0

def checkVersion():
    version_min = int("%4.4i%4.4i%4.4i" % (MIN_VALKKA_VERSION_MAJOR, MIN_VALKKA_VERSION_MINOR, MIN_VALKKA_VERSION_PATCH))
    version = int("%4.4i%4.4i%4.4i" % (VALKKA_VERSION_MAJOR, VALKKA_VERSION_MINOR, VALKKA_VERSION_PATCH))
    if version < version_min:
            print("valkka.nv extension: your libValkka version is too old.  Please update it.")
            return False
    return True


def getVersion():
    return "%i.%i.%i" % (VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH)

