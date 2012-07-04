#Set up distribute (the forked version of
#setuptools) and activate it.  It will only
#install to the source directory if it is
#not already available on the system.
import distribute_setup
distribute_setup.use_setuptools()

#Now do the standard setuptools install
from setuptools import setup, find_packages
setup(
    #Package information
    name = "feynman",
    version = "0.1",
    packages = find_packages("packages"),
    package_data = {
        #Note that the forward slashes
        #are cross-platform (inc. Windows),
        #because distribute automatically
        #converts them.  See the distribute
        #documentation for more information.
        "feynman": ["templates/*.h", 
                    "templates/*.c", 
                    "templates/*.cl"]
    },

    #Package metadata
    author = "Jacob Howard",
    author_email = "jacob@xenoscope.com",
    description = "A package for generating integration code, " \
    "particularly that used to integrate matrix elements.",
    license = "MIT",
    keywords = "monte carlo integration integrate feynman " \
    "amplitude matrix element",
    url = "http://www.xenoscope.com/feynman",

    #Dependencies
    tests_require = ["nose"]
)
