#System modules
from itertools import chain

#Feynman modules
from .parsing import CFunctionDeclaration

class FunctionIntegral(CFunctionDeclaration):
    def __init__(self, integrand, integral_name = None):
        #Validate the integrand
        if not isinstance(integrand, CFunctionDeclaration):
            raise TypeError("The integrand must be a function declaration.")

        #Store the integrand
        self.__integrand = integrand

        #Compute what the new function will be. If the caller has provided 
        #a name, validate it, if not, just do some implementation-defined 
        #name mangling.
        if integral_name:
            if not isinstance(integral_name, basestring):
                raise TypeError("The integral name must be a string.")
        else:
            integral_name = "%s_integral" % integrand.name

        #Generate argument types.  We create upper and lower bounds
        #for each argument in the original integrand.  We use this
        #rather pythonic transform which will essentially do
        #   (1, 2, 3) -> (1, 1, 2, 2, 3, 3)
        argument_types = integrand.argument_types
        argument_types = tuple(chain(*zip(argument_types, argument_types)))

        #Initialize the super-class
        super(FunctionIntegral, self).__init__(integral_name, 
                                               integrand.return_type,
                                               argument_types)

    @property
    def integrand(self):
        return self.__integrand

    def generate_integrator(self):
        pass

class OpenClFunctionIntegral(FunctionIntegral):
    pass
