#System modules
from os.path import join

#Common example modules
from common import example_share_path

#Feynman modules
from feynman.parsing import CFile
from feynman.integration import FunctionIntegral, \
                                GslMonteCarloFunctionIntegrator

#Grab the source code
integrand_header_path = join(example_share_path, "integrand.h")

#Parse the source code
integrand_header = CFile(integrand_header_path)

#Grab the function we're interested in
integrand = integrand_header["unit_cylinder"]
print("The integrand function is: %s" % integrand.signature)

#Create an integral for it
integral = FunctionIntegral(integrand)
print("The integral function is: %s" % integral.signature)

#Add the header dependency for the interal
integral.add_include_dependency("integrand.h")

#Generate code for the function
integrator = GslMonteCarloFunctionIntegrator(integral)

