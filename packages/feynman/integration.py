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

        #Generate argument names.  We suffix _min and _max to each
        #argument if it is already given, otherwise we just name it
        #var_i, where i runs from 1 to N (as in GCC).
        argument_names = []
        for i, name in enumerate(integrand.argument_names):
            arg_index = i + 1
            if name == "":
                argument_names.append("var_%i_min" % arg_index)
                argument_names.append("var_%i_max" % arg_index)
            else:
                argument_names.append("%s_min" % name)
                argument_names.append("%s_max" % name)
        argument_names = tuple(argument_names)

        #Initialize the super-class
        super(FunctionIntegral, self).__init__(integral_name, 
                                               integrand.return_type,
                                               argument_types,
                                               argument_names)

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

    def generate_code(self, header_output = None, code_output = None):
        raise RuntimeError("FunctionIntegrator is an abstract base class.  " \
                           "You must call generate_code from one of its " \
                           "concrete subclasses.")

#Integrator types for integral code generation
GSL_MONTE_CARLO_PLAIN = "GSL_MONTE_CARLO_PLAIN"
GSL_MONTE_CARLO_MISER = "GSL_MONTE_CARLO_MISER"
GSL_MONTE_CARLO_VEGAS = "GSL_MONTE_CARLO_VEGAS"
_ALL_GSL_MONTE_CARLO_TYPES = [
    GSL_MONTE_CARLO_PLAIN,
    GSL_MONTE_CARLO_MISER,
    GSL_MONTE_CARLO_VEGAS
]
_GSL_MONTE_CARLO_TEMPLATES = {
    GSL_MONTE_CARLO_PLAIN: (
        "gsl_plain_monte_carlo.h",
        "gsl_plain_monte_carlo.c"
    ),
    GSL_MONTE_CARLO_MISER: (
        "gsl_miser_monte_carlo.h",
        "gsl_miser_monte_carlo.c"
    ),
    GSL_MONTE_CARLO_VEGAS: (
        "gsl_vegas_monte_carlo.h",
        "gsl_vegas_monte_carlo.c"
    )
}

class GslMonteCarloFunctionIntegrator(FunctionIntegrator):
    def __init__(self, integral, gsl_monte_carlo_type = GSL_MONTE_CARLO_PLAIN):
        #Validate the GSL Monte Carlo type
        if gsl_monte_carlo_type not in _ALL_GSL_MONTE_CARLO_TYPES:
            raise ValueError("Invalid GSL Monte Carlo type specified.")
        self.__gsl_monte_carlo_type = gsl_monte_carlo_type

        #Call the base constructor
        super(GslMonteCarloFunctionIntegrator, self).__init__(integral)

    @property
    def gsl_monte_carlo_type(self):
        return self.__gsl_monte_carlo_type

    def generate_code(self, header_output = None, code_output = None):
        #Grab the template paths
        template_h, template_c = _GSL_MONTE_CARLO_TEMPLATES[self.__gsl_monte_carlo_type]

        #Run the templating engine

class OpenClMonteCarloFunctionIntegrator(FunctionIntegrator):
    def generate_code(self, header_output = None, code_output = None):
        pass
