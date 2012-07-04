#System modules
from os.path import exists, isfile

#C parsing modules
from clang import cindex

class CFunctionDeclaration(object):
    def __init__(self, name, return_type, argument_types, file_path = None, extent = None, text = None):
        #TODO: Type checking

        self.__name = name
        self.__return_type = return_type
        self.__argument_types = argument_types
        self.__file_path = file_path
        self.__extent = extent
        self.__text = text

    @property
    def name(self):
        return self.__name

    @property
    def return_type(self):
        return self.__return_type

    @property
    def argument_types(self):
        return self.__argument_types

    @property
    def file_path(self):
        return self.__file_path

    @property
    def extent(self):
        return self.__extent

    @property
    def has_body(self):
        #Grab the function declaration text
        text = self.text
        if not text:
            return False

        #See if it ends with a semi-colon
        if text.endswith(")") or text.endswith(";"):
            return False

        return True

    @property
    def text(self):
        #See if we have the data in memory
        if self.__text:
            return self.__text

        #Make sure we have a source file
        if not self.__file_path:
            return None
        try:
            source_file = open(self.__file_path, "r")
        except IOError:
            return None

        #Parse the extents
        start_line, start_column, end_line, end_column = self.__extent

        #Read the file
        self.__text = ""
        for i, line in enumerate(source_file):
            i += 1
            if i < start_line:
                continue
            if i < end_line:
                self.__text += line
            elif i == end_line:
                self.__text += line[:end_column]
                break

        #Clear resources
        source_file.close()

        #Cleanup the text
        self.__text = self.__text.strip()

        return self.__text

    @property
    def signature(self):
        fmt = "%s %s(" + ", ".join(("%s",) * len(self.__argument_types)) + ")"
        return fmt % ((self.__return_type, self.__name) + self.__argument_types)

def _name_for_type(t):
    return t.get_canonical().kind.name.lower()
    
def _function_declaration_finder(node, results, file_path):
    if node.kind == cindex.CursorKind.FUNCTION_DECL:
        #Grab the function information
        function_name = node.spelling
        return_type = _name_for_type(node.type.get_result())
        argument_types = tuple([_name_for_type(a_t)
                                for a_t 
                                in node.type.argument_types()])

        #Grab the function location
        start = node.extent.start
        end = node.extent.end
        extent = (start.line, start.column, end.line, end.column)

        #Record the function
        results.append(CFunctionDeclaration(function_name, 
                                            return_type, 
                                            argument_types,
                                            file_path,
                                            extent))

    else:
        #Continue down
        for c in node.get_children():
            _function_declaration_finder(c, results, file_path)

def _find_c_function_declarations(source_file_path):
    #Create an index
    index = cindex.Index.create()

    #Parse the source file
    #HACK: Tell clang to treat the file as Objective-C++,
    #otherwise it translates C++ types (e.g. bool) to their
    #POD equivalents when parsing header files (or maybe it's
    #when it's parsing function prototypes, I can't tell).
    #Anyway, there's no way to force it to use C++, so ObjC++
    #will have to suffice for now.
    translation_unit = index.parse(source_file_path, ["-ObjC++"])

    #Create results
    results = []

    #Traverse the AST
    _function_declaration_finder(translation_unit.cursor, 
                                 results,
                                 source_file_path)

    return results

class CFile(object):
    def __init__(self, file_path):
        #Do basic file validation on file_path
        if not file_path:
            raise ValueError("The file_path argument must not be empty.")
        if not exists(file_path):
            raise ValueError("The file_path argument must point to a file which exists.")
        if not isfile(file_path):
            raise ValueError("The file_path argument must point to a file.")
        self.__file_path = file_path

        #Scan the file for function declarations
        self.__function_declarations = _find_c_function_declarations(file_path)

    @property
    def file_path(self):
        return self.__file_path

    @property
    def function_declarations(self):
        return self.__function_declarations
