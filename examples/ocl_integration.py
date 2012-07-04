#System modules
from os.path import join

#Common example modules
from common import example_share_path

#Feynman modules
from feynman.parsing import CFile

#Grab the source code
integrand_h = join(example_share_path, "integrand.h")
integrand_c = join(example_share_path, "integrand.c")

#Parse the source code
integrand_h_code = CFile(integrand_h)
integrand_c_code = CFile(integrand_c)
