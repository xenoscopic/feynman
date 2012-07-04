#Feynman modules
from feynman import parsing, integration

#System modules
from os.path import join

#Testing modules
from common import resources_path

def test_integral_generation():
    #Grab the resources path
    resource_directory_path = resources_path()

    #Grab the input code path
    input_code_path = join(resource_directory_path, "parsing_test_code.cl")

    #Parse the input code
    input_code = parsing.CFile(input_code_path)
    
    #Make sure there is at least one function detected
    assert(len(input_code.function_declarations) > 0)

    #Loop through and generate an integral for each integrand,
    #validating that the integrals make sense.
    for integrand in input_code.function_declarations:
        #Create an integral
        integral = integration.FunctionIntegral(integrand)

        #Make sure the return types match
        assert(integral.return_type == integrand.return_type)

        #Make sure the argumet types make sense.  There should
        #be twice as many argument_types for the integral as the
        #integrand (for lower and upper limits), and the types
        #should match up.
        assert(len(integrand.argument_types) > 0)
        assert(len(integral.argument_types) == (2 * len(integrand.argument_types)))
        arg_i = 0
        for arg_type in integrand.argument_types:
            for i in xrange(0, 2):
                assert(integral.argument_types[arg_i + i])
            arg_i += 2
