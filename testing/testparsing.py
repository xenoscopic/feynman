#Common testing modules
from common import testing_resource_path

#Feynman modules
from feynman import parsing

#System modules
from os.path import join
import json

def _compare_parsing(file_name):
    #Grab the input code path
    code_file_path = join(testing_resource_path, file_name)

    #Grab the validation information path
    validation_information = join(testing_resource_path, "parsing_test_validation.json")

    #Parse the input code
    code_file = parsing.CFile(code_file_path)

    #Parse the validation code
    validation_file = open(validation_information, "r")
    validation_data = validation_file.read()
    validation_file.close()
    validation_data = json.loads(validation_data)

    #Make sure there is at least one function detected
    assert(len(code_file.function_declarations) > 0)

    #Make sure the number of function declarations
    #matches the number expected
    assert(len(code_file.function_declarations) == len(validation_data))

    #Validate the parse data against reference metadata
    for data in validation_data:
        function = code_file[data["name"]]
        assert(function != None)
        assert(function.name == data["name"])
        assert(function.return_type == data["return_type"])
        assert(function.argument_types == tuple(data["argument_types"]))
        assert(len(function.argument_names) == len(function.argument_types))

        #Make sure argument names, if given, match the reference
        for arg_name, ref_arg_name in zip(function.argument_names, \
                                          tuple(data["argument_names"])):
            if arg_name != "":
                assert(arg_name == ref_arg_name)

    #Make sure bad function names result in none
    assert(code_file["1a"] == None)
    assert(code_file[None] == None)
    assert(code_file[1] == None)

def test_header_parsing():
    _compare_parsing("parsing_test_code.h")

def test_source_parsing():
    _compare_parsing("parsing_test_code.cl")

def test_body_parsing():
    #Grab the input code path
    header_path = join(testing_resource_path, "parsing_test_code.h")
    source_path = join(testing_resource_path, "parsing_test_code.cl")

    #Parse the input code
    header_file = parsing.CFile(header_path)
    source_file = parsing.CFile(source_path)

    #Make sure headers don't have bodies
    for decl in header_file.function_declarations:
        assert(not decl.has_body)

    #Make sure source files do!
    for decl in source_file.function_declarations:
        assert(decl.has_body)

def test_include_dependencies():
    #Grab the input code path
    header_path = join(testing_resource_path, "parsing_test_code.h")

    #Parse the input code
    header_file = parsing.CFile(header_path)

    #Go through and make sure includes work for
    #each function.
    for decl in header_file.function_declarations:
        #Make sure there are no dependencies to start with
        assert(len(decl.include_dependencies) == 0)

        #Generate a dependency
        decl.add_source_file_dependency(1)

        #Make sure there is only one
        assert(len(decl.include_dependencies) == 1)

        #Make sure it is correct
        assert(decl.include_dependencies[0] == "parsing_test_code.h")
