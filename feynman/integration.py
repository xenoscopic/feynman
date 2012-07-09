#System modules
import sys
from itertools import chain
from pkg_resources import resource_string
from os.path import basename

#Feynman modules
from .parsing import CFunctionDeclaration
from .common import validate_code_string

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
            validate_code_string(integral_name)
        else:
            integral_name = "%s_integral" % integrand.name

        #Generate argument types.  We create upper and lower bounds
        #for each argument in the original integrand.  We use this
        #rather pythonic transform which will essentially do
        #   (1, 2, 3) -> (1, 1, 2, 2, 3, 3)
        #We also create a last argument which will return the error
        #into a pointer (if provided).
        argument_types = integrand.argument_types
        argument_types = tuple(chain(*zip(argument_types, argument_types)))
        argument_types += (integrand.return_type + " *",)

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
        argument_names += ("error",)

        #Generate argument default values
        argument_default_values = ("",) * (len(argument_names) - 1) + ("NULL",)

        #Initialize the super-class
        super(FunctionIntegral, self).__init__(integral_name, 
                                               integrand.return_type,
                                               argument_types,
                                               argument_names,
                                               argument_default_values)

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
    def __init__(self, integral, header_include_name = None):
        #Validate the integral
        if not isinstance(integral, FunctionIntegral):
            raise TypeError("The integral must be a FunctionIntegral.")
        self.__integral = integral

        #Validate the file name
        if header_include_name != None:
            validate_code_string(header_include_name)
        self.__header_include_name = header_include_name

    @property
    def integral(self):
        return self.__integral

    @property
    def header_include_name(self):
        return self.__header_include_name

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
_GSL_BASE_MONTE_CARLO_HEADER = "gsl_base_monte_carlo.h"
_GSL_BASE_MONTE_CARLO_SOURCE = "gsl_base_monte_carlo.c"
_GSL_MONTE_CARLO_TEMPLATES = {
    GSL_MONTE_CARLO_PLAIN: "gsl_plain_monte_carlo.c",
    GSL_MONTE_CARLO_MISER: "gsl_miser_monte_carlo.c",
    GSL_MONTE_CARLO_VEGAS: "gsl_vegas_monte_carlo.c"
}
_MONTE_CARLO_TEMPLATE_PATH = "templates"

class GslMonteCarloFunctionIntegrator(FunctionIntegrator):
    def __init__(self, 
                 integral, 
                 header_include_name = None, 
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
        super(GslMonteCarloFunctionIntegrator, self).__init__(integral, header_include_name)

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
        integration_template_name = _GSL_MONTE_CARLO_TEMPLATES[self.__gsl_monte_carlo_type]

        #Compute template paths using the pkg_resources API.
        #NOTE: We use "/" as the path separator here, this
        #is because these are not actually file system paths,
        #and the pkg_resources API will convert them to the 
        #appropriate separators in a cross-platform manner.
        header_template = resource_string(__name__, 
                                          "/".join([
                                          _MONTE_CARLO_TEMPLATE_PATH, 
                                          _GSL_BASE_MONTE_CARLO_HEADER
                                          ])
                                         )
        source_template = resource_string(__name__, 
                                          "/".join([
                                          _MONTE_CARLO_TEMPLATE_PATH, 
                                          _GSL_BASE_MONTE_CARLO_SOURCE
                                          ])
                                         )
        integration_template = resource_string(__name__, 
                                               "/".join([
                                               _MONTE_CARLO_TEMPLATE_PATH, 
                                               integration_template_name
                                               ])
                                               )

        #Compute what the include header should be
        if self.header_include_name:
            header_include_name = self.header_include_name
        elif isinstance(header_output, basestring):
            header_include_name = basename(header_output)
        else:
            header_include_name = "Integral.h"

        #Create the data
        template_data = {
            "integral": self.integral,
            "header_include_name": header_include_name,
            "n_calls": self.n_calls
        }

        #Load the templates and fill with data
        integration_template = Template(integration_template, searchList = [template_data])
        template_data["integration_template"] = str(integration_template)
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
