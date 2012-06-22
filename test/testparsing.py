#Feynman modules
from feynman import parsing

#System modules
from os.path import join
import json

#Testing modules
from common import resources_path

def test_parsing():
    #Grab the resources path
    resource_directory_path = resources_path()

    #Grab the input code path
    input_code = join(resource_directory_path, "parsing_test_code.cl")

    #Grab the validation information path
    validation_information = join(resource_directory_path, "parsing_test_validation.json")

    #Parse the input code
    function_declarations = parsing.find_c_function_declarations(input_code)

    #Parse the validation code
    validation_file = open(validation_information, "r")
    validation_data = validation_file.read()
    validation_file.close()
    validation_data = json.loads(validation_data)

    #Make sure there is at least one function detected
    assert(len(function_declarations) > 0)

    #Make sure the number of function declarations
    #matches the number expected
    assert(len(function_declarations) == len(validation_data))

    #Loop through and make sure the parsing worked
    for decl, data in zip(function_declarations, validation_data):
        assert(decl.name == data["name"])
        assert(decl.return_type == data["return_type"])
        assert(decl.argument_types == tuple(data["argument_types"]))
