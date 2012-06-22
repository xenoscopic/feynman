Table of Contents
=================
1. Requirements
2. Testing


Requirements
============
-Bourne shell interpreter
-curl OR wget (for installing external dependencies)
-python with the following modules:
    -numpy
    -pyopencl
    -clang (can be installed by using built-in externals)
    -nose (for testing)


Testing
=======
Unit testing is performed with the Python module "nose."  After installing nose, you can run tests with:

$ nosetests PATH/TO/FEYNMAN/SOURCE

All tests should complete successfully.
