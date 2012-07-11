#!/usr/bin/env python

#System modules
import sys
import os

#Argument parsing modules
import argparse

#Feynman modules
from feynman.parsing import CFile
from feynman.integration import GslMonteCarloFunctionIntegrator, \
                                OpenClMonteCarloFunctionIntegrator, \
                                GSL_MONTE_CARLO_PLAIN, \
                                GSL_MONTE_CARLO_MISER, \
                                GSL_MONTE_CARLO_VEGAS, \
                                OPENCL_MONTE_CARLO_PLAIN

#Helper functions
def parse_arguments():
    #Create the argument parser
    parser = argparse.ArgumentParser(description = "Feynman Integrator Generator")
    parser.add_argument("-v", 
                        "--verbose", 
                        dest = "verbose", 
                        action = "store_true", 
                        help = "Show verbose output.")
    parser.add_argument("-i", 
                        "--integrand-file", 
                        dest = "integrand_file", 
                        required = True, 
                        metavar = "FILE", 
                        help = "The input file containing the integrand.  If using " \
                               "the OpenCL backend, the file must contain the body " \
                               "of the integrand function.")
    parser.add_argument("-I",
                        "--integrand-name",
                        dest = "integrand_name",
                        required = True,
                        metavar = "FUNCTION",
                        help = "The function contained in the input file to be integrated.")
    parser.add_argument("-d",
                        "--dependencies",
                        dest = "dependencies",
                        required = False,
                        default = [],
                        metavar = "DEPENDENCY",
                        nargs = '*',
                        help = "A list of files to include with preprocessor directives" \
                               "in the generated code.  These should be just file names " \
                               "or relative paths.  Whatever is included here will be " \
                               "put directly into an include directive, e.g. " \
                               "#include \"DEPENDENCY\".")
    parser.add_argument("-o",
                        "--output-file-base",
                        dest = "output_file_base",
                        required = False,
                        default = None,
                        metavar = "FILE",
                        help = "The base path of the output file.  This path will be " \
                        "used to compute .h and .c output paths if they are not " \
                        "specified manually.")
    parser.add_argument("-H",
                        "--header-output-path",
                        dest = "header_output_path",
                        required = False,
                        default = None,
                        metavar = "HEADER_PATH",
                        help = "The specific output path of the header.  This will " \
                               "override any header path computed from output-file-base.")
    parser.add_argument("-S",
                        "--source-output-path",
                        dest = "source_output_path",
                        required = False,
                        default = None,
                        metavar = "SOURCE_PATH",
                        help = "The specific output path of the source.  This will " \
                               "override any header path computed from output-file-base.")
    parser.add_argument("-N",
                        "--integrator-name",
                        dest = "integrator_name",
                        required = False,
                        default = None,
                        metavar = "FUNCTION",
                        help = "The name for the integrator class in the output file.")
    parser.add_argument("-P",
                        "--primary-header-include",
                        dest = "primary_header_include",
                        required = False,
                        default = None,
                        metavar = "FILE",
                        help = "This option allows the user to override the name of " \
                               "the main included header file in the generated source " \
                               "file.  By default, this program will simply take " \
                               "the tail of the specified output path for the include " \
                               "directive.  For instance, if the output header is " \
                               "/home/user/integral1.h, the included file will be " \
                               "integral1.h.  " \
                               "However, it may be the case that the user wishes to " \
                               "relocate the header file and include it relative to " \
                               "some other path.  You can use this option to override " \
                               "the default header path, to make it something like " \
                               "#include \"integrals/integral1.h\".  To do this, simply " \
                               "specify what should appear in the quotation marks of " \
                               "the include directive.")
    parser.add_argument("-b",
                        "--backend",
                        dest = "backend",
                        required = False,
                        default = "gsl-plain",
                        metavar = "BACKEND",
                        help = "The integrator backend to use.  Available options are " \
                               "gsl-plain, gsl-miser, gsl-vegas, and opencl-plain.  Note " \
                               "that while you are not required to have either GSL or " \
                               "OpenCL on your system to generate integration code, " \
                               "you will need the respective package available to " \
                               "compile and execute the code.")

    #Run the parser
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_arguments()
    
    #Print program information
    if args.verbose:
        print("Feynman Integration Code Generator")

    #Open the input file
    if args.verbose:
        print("Loading code from: %s" % args.integrand_file)
    input_file = CFile(args.integrand_file)

    #Grab the function we're interested in
    if args.verbose:
        print("Using function: %s" % args.integrand_name)
    integrand = input_file[args.integrand_name]
    if not integrand:
        print("Unable to find integrand: %s" % args.integrand_name)
        sys.exit(1)
    if args.verbose:
        print("Integrand signature:")
        print("\t%s" % integrand.signature)

    #Add file dependencies
    for dependency in args.dependencies:
        if args.verbose:
            print("Adding dependency for \"%s\"" % dependency)
        integrand.add_include_dependency(dependency)

    #Create the correct code generator
    if args.backend not in ["gsl-plain", 
                            "gsl-miser", 
                            "gsl-vegas", 
                            "opencl-plain"]:
        print("Invalid backend specified: %s" % args.backend)
        sys.exit(1)
    if args.verbose:
        print("Using backend: %s" % args.backend)
    is_gsl = args.backend.startswith("gsl")
    if is_gsl:
        if args.backend.endswith("plain"):
            gsl_type = GSL_MONTE_CARLO_PLAIN
        elif args.backend.endswith("miser"):
            gsl_type = GSL_MONTE_CARLO_MISER
        elif args.backend.endswith("vegas"):
            gsl_type = GSL_MONTE_CARLO_VEGAS
        integrator = GslMonteCarloFunctionIntegrator(integrand, 
                                                     args.integrator_name,
                                                     gsl_monte_carlo_type = gsl_type)
    else:
        if args.backend.endswith("plain"):
            opencl_type = OPENCL_MONTE_CARLO_PLAIN
        integrator = OpenClMonteCarloFunctionIntegrator(integrand,
                                                        args.integrator_name,
                                                        opencl_monte_carlo_type = opencl_type)
    if args.verbose:
        print("Integral signature:")
        print("\t%s" % integrator.evaluation_function.signature)

    #Compute output paths
    if args.header_output_path != None:
        output_header = args.header_output_path
    elif args.output_file_base != None:
        output_header = args.output_file_base + ".h"
    else:
        raise RuntimeError("Unable to compute header output path.  " \
                           "You must specify either header-output-path " \
                           "or output-file-base from the command line.")
    if args.source_output_path != None:
        output_source = args.source_output_path
    elif args.output_file_base != None:
        output_source = args.output_file_base + ".c"
    else:
        raise RuntimeError("Unable to compute source output path.  " \
                           "You must specify either source-output-path " \
                           "or output-file-base from the command line.")
    if args.verbose:
        print("Header output path: %s" % output_header)
        print("Source output path: %s" % output_source)

    #Create intermediate directories
    header_dir = os.path.dirname(output_header)
    source_dir = os.path.dirname(output_source)
    if not os.path.isdir(header_dir):
        os.makedirs(header_dir)
    if not os.path.isdir(source_dir):
        os.makedirs(source_dir)

    #Generate the code!
    integrator.generate_code(output_header, 
                             output_source,
                             primary_header_include = args.primary_header_include)
    if args.verbose:
        print("Code successfully generated!")
