#System modules
from itertools import chain

#Feynman modules
from .parsing import CFunctionDeclaration

#Integrator types for integral code generation
INTEGRATOR_TYPE_GSL_MONTE_CARLO_PLAIN = "INTEGRATOR_TYPE_GSL_MONTE_CARLO_PLAIN"
INTEGRATOR_TYPE_GSL_MONTE_CARLO_MISER = "INTEGRATOR_TYPE_GSL_MONTE_CARLO_MISER"
INTEGRATOR_TYPE_GSL_MONTE_CARLO_VEGAS = "INTEGRATOR_TYPE_GSL_MONTE_CARLO_VEGAS"
INTEGRATOR_TYPE_OPENCL_MONTE_CARLO_PLAIN = "INTEGRATOR_TYPE_OPENCL_MONTE_CARLO_PLAIN"

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

    @property
    def fix_parameter(self, variable_name_or_index, value):
        #TODO!
        pass

    @property
    def transform_variable(self, variable_name_or_index):
        #TODO!
        pass

class FunctionIntegrator(object):
    def __init__(self, integral):
        #Validate the integral
        if not isinstance(integral, FunctionIntegral):
            raise TypeError("The integral must be a FunctionIntegral.")
        
        #Store the integral
        self.__integral = integral

    @property
    def integral(self):
        return self.__integral

    def generate_code(self, output = None):
        raise RuntimeError("FunctionIntegrator is an abstract base class.  " \
                           "You must call generate_code from one of its " \
                           "concrete subclasses.")

class GslPlainMonteCarloFunctionIntegrator(FunctionIntegrator):
    def generate_code(self, output = None):
        pass

class GslVegasMonteCarloFunctionIntegrator(FunctionIntegrator):
    def __init__(self, integral, pdfs):
        pass

    def generate_code(self, output = None):
        pass

class GslMiserMonteCarloFunctionIntegrator(FunctionIntegrator):
    def generate_code(self, output = None):
        pass

class OpenClMonteCarloFunctionIntegrator(FunctionIntegrator):
    def generate_code(self, output = None, target_device = None):
        pass



