#Feynman modules
from feynman import parsing

#System modules
from os.path import join
import json

#Testing modules
from common import resources_path

def _compare_parsing(file_name):
    #Grab the resources path
    resource_directory_path = resources_path()

    #Grab the input code path
    input_file_path = join(resource_directory_path, file_name)

    #Grab the validation information path
    validation_information = join(resource_directory_path, "parsing_test_validation.json")

    #Parse the input code
    input_file = parsing.CFile(input_file_path)

    #Parse the validation code
    validation_file = open(validation_information, "r")
    validation_data = validation_file.read()
    validation_file.close()
    validation_data = json.loads(validation_data)

    #Make sure there is at least one function detected
    assert(len(input_file.function_declarations) > 0)

    #Make sure the number of function declarations
    #matches the number expected
    assert(len(input_file.function_declarations) == len(validation_data))

    #Loop through and make sure the parsing worked
    for decl, data in zip(input_file.function_declarations, validation_data):
        assert(decl.name == data["name"])
        assert(decl.return_type == data["return_type"])
        assert(decl.argument_types == tuple(data["argument_types"]))

def test_header_parsing():
    _compare_parsing("parsing_test_code.h")

def test_source_parsing():
    _compare_parsing("parsing_test_code.cl")

def test_body_parsing():
    #Grab the resources path
    resource_directory_path = resources_path()

    #Grab the input code path
    header_path = join(resource_directory_path, "parsing_test_code.h")
    source_path = join(resource_directory_path, "parsing_test_code.cl")

    #Parse the input code
    header_file = parsing.CFile(header_path)
    source_file = parsing.CFile(source_path)

    #Make sure headers don't have bodies
    for decl in header_file.function_declarations:
        assert(not decl.has_body)

    #Make sure source files do!
    for decl in source_file.function_declarations:
        assert(decl.has_body)
