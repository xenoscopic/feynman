#System modules
from os.path import join, dirname, realpath
import sys

#Grab paths
source_path = dirname(dirname(realpath(__file__)))
share_path = join(source_path, "examples/share")

#Feynman modules
sys.path.append(source_path)
from feynman.parsing import CFile

#Grab the source code
integrand_h = join(share_path, "integrand.h")
integrand_c = join(share_path, "integrand.c")

print(integrand_h)
print(integrand_c)

#Parse the source code
integrand_h_code = CFile(integrand_h)
integrand_c_code = CFile(integrand_c)

#Print out functions
for function in integrand_h_code.function_declarations:
    print(function.extent)
    print(function.has_body)
    print(function.text)