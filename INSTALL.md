Table of Contents
=================
1. Dependencies
2. Testing


Dependencies
============
-[Required] Bourne shell interpreter
-[Required] curl OR wget (for installing external dependencies)
-[Required] LLVM/Clang (for parsing, can be installed automatically using the externals script)
-[Required] python 2.6+
-[Required] python modules:
    -clang (can be installed automatically using the externals script)
-[Optional] python modules:
    -numpy (for OpenCL integration support)
    -pyopencl (for OpenCL integration support)
    -nose (for running the built-in tests)
-[Optional] OpenCL (at least one platform for OpenCL integration support)


Testing
=======
Unit testing is performed with the Python module "nose."  After installing nose, you can run tests with:

$ nosetests PATH/TO/FEYNMAN/SOURCE

All tests should complete successfully.
