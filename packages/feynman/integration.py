#System modules
import sys
from itertools import chain
from pkg_resources import resource_string
import re

#Feynman modules
from .parsing import CFunctionDeclaration

#Cheetah modules
from Cheetah.Template import Template

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

    @property
    def n_dimensions(self):
        #TODO: Implement corrections from variable transformations
        return len(self.argument_types) / 2

class FunctionIntegrator(object):
    def __init__(self, integral, file_name = "Integrate"):
        #Validate the integral
        if not isinstance(integral, FunctionIntegral):
            raise TypeError("The integral must be a FunctionIntegral.")
        self.__integral = integral

        #Validate the file name
        if not isinstance(file_name, basestring):
            raise TypeError("The file name must be a string.")
        valid_characters = re.compile("[\W_]+")
        file_name = valid_characters.sub('', file_name)
        if file_name == "":
            raise ValueError("The file name must contain " \
                             "only alphanumeric characters " \
                             "and must contain at least one " \
                             "of them.")
        self.__file_name = file_name

    @property
    def integral(self):
        return self.__integral

    @property
    def file_name(self):
        return self.__file_name

    def generate_code(self, header_output = sys.stdout, source_output = sys.stdout):
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
_MONTE_CARLO_TEMPLATE_PATH = "templates"

class GslMonteCarloFunctionIntegrator(FunctionIntegrator):
    def __init__(self, 
                 integral, 
                 file_name = "Integrate", 
                 gsl_monte_carlo_type = GSL_MONTE_CARLO_PLAIN,
                 n_calls = 500000):
        #Validate the GSL Monte Carlo type
        if gsl_monte_carlo_type not in _ALL_GSL_MONTE_CARLO_TYPES:
            raise ValueError("Invalid GSL Monte Carlo type specified.")
        self.__gsl_monte_carlo_type = gsl_monte_carlo_type

        #Validate the number of calls
        if not isinstance(n_calls, int) or n_calls < 1:
            raise TypeError("Number of calls must be an integer >= 1.")
        self.__n_calls = n_calls

        #Call the base constructor
        super(GslMonteCarloFunctionIntegrator, self).__init__(integral, file_name)

    @property
    def gsl_monte_carlo_type(self):
        return self.__gsl_monte_carlo_type

    @property
    def n_calls(self):
        return self.__n_calls

    def generate_code(self, 
                      header_output = sys.stdout, 
                      source_output = sys.stdout):
        #Grab the template names
        header_template_name, source_template_name \
            = _GSL_MONTE_CARLO_TEMPLATES[self.__gsl_monte_carlo_type]

        #Compute template paths using the pkg_resources API.
        #NOTE: We use "/" as the path separator here, this
        #is because these are not actually file system paths,
        #and the pkg_resources API will convert them to the 
        #appropriate separators in a cross-platform manner.
        header_template = resource_string(__name__, 
                                          "/".join([
                                          _MONTE_CARLO_TEMPLATE_PATH, 
                                          header_template_name
                                          ])
                                         )
        source_template = resource_string(__name__, 
                                          "/".join([
                                          _MONTE_CARLO_TEMPLATE_PATH, 
                                          source_template_name
                                          ])
                                         )

        #Create the data
        template_data = {
            "integral": self.integral,
            "file_name": self.file_name,
            "n_calls": self.n_calls
        }

        #Load the templates and fill with data
        header_template = Template(header_template, searchList = [template_data])
        source_template = Template(source_template, searchList = [template_data])

        #Spit out the templates.  For each we first
        #check if the output has a 'write' method 
        #and then call that, or we treat it as a file
        #path.  I considered checking for 
        #isinstance(output, io.IOBase), but this wouldn't
        #work for a lot of things (sys.stdout, StringIO),
        #so I'm just doing this for now.
        if hasattr(header_output, "write"):
            header_output.write(str(header_template))
        else:
            with open(header_output, "w") as f:
                f.write(str(header_template))
        if hasattr(source_output, "write"):
            source_output.write(str(source_template))
        else:
            with open(source_output, "w") as f:
                f.write(str(source_template))


class OpenClMonteCarloFunctionIntegrator(FunctionIntegrator):
    def generate_code(self, 
                      header_output = sys.stdout, 
                      source_output = sys.stdout):
        pass
