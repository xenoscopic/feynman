#C parsing modules
from clang import cindex

class CFunctionDeclaration(object):
    def __init__(self, name, return_type, argument_types):
        self.__name = name
        self.__return_type = return_type
        self.__argument_types = argument_types

    @property
    def name(self):
        return self.__name

    @property
    def return_type(self):
        return self.__return_type

    @property
    def argument_types(self):
        return self.__argument_types

def _function_declaration_finder(node, results):
    if node.kind == cindex.CursorKind.FUNCTION_DECL:
        #Grab the function information
        function_name = node.spelling
        return_type = node.type.get_result().kind.spelling.lower()
        argument_types = tuple([a_t.kind.spelling.lower() 
                                for a_t 
                                in node.type.argument_types()])

        #Record the function
        results.append(CFunctionDeclaration(function_name, 
                                            return_type, 
                                            argument_types))

    else:
        #Continue down
        for c in node.get_children():
            _function_declaration_finder(c, results)

def find_c_function_declarations(source_file_path):
    #Create an index
    index = cindex.Index.create()

    #Parse the source file
    translation_unit = index.parse(source_file_path)

    #Create results
    results = []

    #Traverse the AST
    _function_declaration_finder(translation_unit.cursor, results)

    return results
