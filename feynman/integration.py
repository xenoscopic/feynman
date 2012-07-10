#System modules
import sys
from itertools import chain
from pkg_resources import resource_string
from os.path import basename

#Feynman modules
from .parsing import CFunctionDeclaration
from .common import validate_code_string, underscore_to_camel_case

#Cheetah modules
from Cheetah.Template import Template

class FunctionIntegrator(object):
    def __init__(self, integrand, name = None):
        #Validate the integrand
        if not isinstance(integrand, CFunctionDeclaration):
            raise TypeError("The integrand must be a CFunctionDeclaration.")
        self.__integrand = integrand

        #Validate the integrator name, if provided,
        #otherwise, create one based on the integrand
        #name
        if name != None:
            validate_code_string(name)
        else:
            name = underscore_to_camel_case(integrand.name) + "Integrator"
        self.__name = name

        #Calculate the number of dimensions
        self.__n_dimensions = len(integrand.argument_types)

        #Create a function declaration for the evaluation
        #method
        evaluation_function_name = "evaluate"

        #Generate argument types.  We create upper and lower bounds
        #for each argument in the original integrand.  We use this
        #rather pythonic transform which will essentially do
        #   (1, 2, 3) -> (1, 1, 2, 2, 3, 3)
        #We also create a last argument which will return the error
        #into a pointer (if provided).
        evaluation_argument_types = integrand.argument_types
        evaluation_argument_types = tuple(chain(*zip(evaluation_argument_types, 
                                                     evaluation_argument_types)))
        evaluation_argument_types += (integrand.return_type + " *",)

        #Generate argument names.  We suffix _min and _max to each
        #argument if it is already given, otherwise we just name it
        #var_i, where i runs from 1 to N (as in GCC).
        evaluation_argument_names = []
        for i, name in enumerate(integrand.argument_names):
            arg_index = i + 1
            if name == "":
                evaluation_argument_names.append("var_%i_min" % arg_index)
                evaluation_argument_names.append("var_%i_max" % arg_index)
            else:
                evaluation_argument_names.append("%s_min" % name)
                evaluation_argument_names.append("%s_max" % name)
        evaluation_argument_names = tuple(evaluation_argument_names)
        evaluation_argument_names += ("error",)

        #Generate argument default values
        evaluation_argument_default_values = ("",) * (len(evaluation_argument_names) - 1) + ("NULL",)

        #Initialize the super-class
        self.__evaluation_function = CFunctionDeclaration(evaluation_function_name, 
                                                          integrand.return_type,
                                                          evaluation_argument_types,
                                                          evaluation_argument_names,
                                                          evaluation_argument_default_values)

    @property
    def integrand(self):
        return self.__integrand

    @property
    def name(self):
        return self.__name

    @property
    def n_dimensions(self):
        return self.__n_dimensions

    @property
    def evaluation_function(self):
        return self.__evaluation_function

    def generate_code(self, 
                      header_output = sys.stdout, 
                      source_output = sys.stdout,
                      primary_header_include = None):
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
_GSL_BASE_MONTE_CARLO_HEADER = "GslBaseMonteCarlo.h"
_GSL_BASE_MONTE_CARLO_SOURCE = "GslBaseMonteCarlo.cpp"
_GSL_MONTE_CARLO_TEMPLATES = {
    GSL_MONTE_CARLO_PLAIN: "GslPlainMonteCarlo.cpp",
    GSL_MONTE_CARLO_MISER: "GslMiserMonteCarlo.cpp",
    GSL_MONTE_CARLO_VEGAS: "GslVegasMonteCarlo.cpp"
}
_GSL_MONTE_CARLO_INCLUDES = {
    GSL_MONTE_CARLO_PLAIN: "gsl/gsl_monte_plain.h",
    GSL_MONTE_CARLO_MISER: "gsl/gsl_monte_miser.h",
    GSL_MONTE_CARLO_VEGAS: "gsl/gsl_monte_vegas.h"
}
_GSL_MONTE_CARLO_TEMPLATE_PATH = "templates"

class GslMonteCarloFunctionIntegrator(FunctionIntegrator):
    def __init__(self, 
                 integral, 
                 name = None, 
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
        super(GslMonteCarloFunctionIntegrator, self).__init__(integral, name)

    @property
    def gsl_monte_carlo_type(self):
        return self.__gsl_monte_carlo_type

    @property
    def n_calls(self):
        return self.__n_calls

    def generate_code(self, 
                      header_output = sys.stdout, 
                      source_output = sys.stdout,
                      primary_header_include = None):
        #Grab the template names
        gsl_method_template = _GSL_MONTE_CARLO_TEMPLATES[self.__gsl_monte_carlo_type]
        gsl_method_header = _GSL_MONTE_CARLO_INCLUDES[self.__gsl_monte_carlo_type]

        #Compute template paths using the pkg_resources API.
        #NOTE: We use "/" as the path separator here, this
        #is because these are not actually file system paths,
        #and the pkg_resources API will convert them to the 
        #appropriate separators in a cross-platform manner.
        header_template = resource_string(__name__, 
                                          "/".join([
                                          _GSL_MONTE_CARLO_TEMPLATE_PATH, 
                                          _GSL_BASE_MONTE_CARLO_HEADER
                                          ])
                                         )
        source_template = resource_string(__name__, 
                                          "/".join([
                                          _GSL_MONTE_CARLO_TEMPLATE_PATH, 
                                          _GSL_BASE_MONTE_CARLO_SOURCE
                                          ])
                                         )
        method_template = resource_string(__name__, 
                                          "/".join([
                                          _GSL_MONTE_CARLO_TEMPLATE_PATH, 
                                          gsl_method_template
                                          ])
                                         )

        #Compute what the primary include header and include
        #guard should be.
        if not primary_header_include:
            if isinstance(header_output, basestring):
                primary_header_include = basename(header_output)
            else:
                primary_header_include = "Integral.h"
        include_guard = primary_header_include.replace("/", "_") \
                                              .replace("\\", "_") \
                                              .replace(".", "_") \
                                              .upper()

        #Create the data
        template_data = {
            "integrator": self,
            "primary_header_include": primary_header_include,
            "include_guard": include_guard,
            "gsl_method_header": gsl_method_header,
            "n_calls": self.n_calls
        }

        #Load the templates and fill with data
        method_template = Template(method_template, searchList = [template_data])
        template_data["method_template"] = str(method_template)
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

_OPENCL_MONTE_CARLO_HEADER = "OpenClMonteCarlo.h"
_OPENCL_MONTE_CARLO_SOURCE = "OpenClMonteCarlo.cpp"
_OPENCL_RANLUX_SOURCE = "ranluxcl.cl"
_OPENCL_CARLO_TEMPLATE_PATH = "templates"

class OpenClMonteCarloFunctionIntegrator(FunctionIntegrator):
    def generate_code(self, 
                      header_output = sys.stdout, 
                      source_output = sys.stdout,
                      primary_header_include = None):
        #Compute template paths using the pkg_resources API.
        #NOTE: We use "/" as the path separator here, this
        #is because these are not actually file system paths,
        #and the pkg_resources API will convert them to the 
        #appropriate separators in a cross-platform manner.
        header_template = resource_string(__name__, 
                                          "/".join([
                                          _OPENCL_CARLO_TEMPLATE_PATH, 
                                          _OPENCL_MONTE_CARLO_HEADER
                                          ])
                                         )
        source_template = resource_string(__name__, 
                                          "/".join([
                                          _OPENCL_CARLO_TEMPLATE_PATH, 
                                          _OPENCL_MONTE_CARLO_SOURCE
                                          ])
                                         )
        ranlux_template = resource_string(__name__, 
                                          "/".join([
                                          _OPENCL_CARLO_TEMPLATE_PATH, 
                                          _OPENCL_RANLUX_SOURCE
                                          ])
                                         )

        #Compute what the primary include header and include
        #guard should be.
        if not primary_header_include:
            if isinstance(header_output, basestring):
                primary_header_include = basename(header_output)
            else:
                primary_header_include = "Integral.h"
        include_guard = primary_header_include.replace("/", "_") \
                                              .replace("\\", "_") \
                                              .replace(".", "_") \
                                              .upper()

        #Create the data
        template_data = {
            "integrator": self,
            "primary_header_include": primary_header_include,
            "include_guard": include_guard,
            "ranlux_template": str(ranlux_template)
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
