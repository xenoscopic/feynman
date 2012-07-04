#System modules
from os.path import join, dirname, realpath
import sys

#Figure out the path to the distribution source code
_distribution_path = dirname(dirname(realpath(__file__)))

#Figure out the path for the modules
_module_path = join(_distribution_path, "packages")

#Figure out the path for the example resources
example_share_path = join(_distribution_path, "examples/share")

#Try to import the feynman module from the system,
#and if it isn't there, add the distribution path
#to the python search path.
try:
    import feynman
except ImportError:
    print("Unable to find the feynman module on the system, " \
          "just running it from the source distribution.")
    sys.path.append(_module_path)
