#Common testing modules
import common

#Feynman modules
from feynman.opencl import OpenClContext

def test_feynman_opencl_context():
    #Create a context, which will test that
    #there is an OpenCL platform accessible
    #and available.
    context = OpenClContext()
    print(dir(context.device))
