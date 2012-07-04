#System modules
from os.path import exists, isfile, split, join
from itertools import chain

#C parsing modules
from clang import cindex

class CFunctionDeclaration(object):
    def __init__(self, name, return_type, argument_types, argument_names, file_path = None, extent = None, text = None):
        #TODO: Type checking

        self.__name = name
        self.__return_type = return_type
        self.__argument_types = argument_types
        self.__argument_names = argument_names
        self.__file_path = file_path
        self.__extent = extent
        self.__text = text

        #Create include dependencies
        self.__include_dependencies = []

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
    def argument_names(self):
        return self.__argument_names

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
        #Do the argument formatting
        arg_format_list = []
        arg_format_tuple = ()
        for arg_type, arg_name in zip(self.__argument_types, \
                                      self.__argument_names):
            arg_format_tuple += (arg_type,)
            if arg_name != "":
                arg_format_tuple += (arg_name,)
                arg_format_list.append("%s %s")
            else:
                arg_format_list.append("%s")
        arg_format_string = ", ".join(arg_format_list)

        fmt = "%s %s(" + arg_format_string + ")"
        return fmt % ((self.__return_type, self.__name) + arg_format_tuple)

    def add_include_dependency(self, dependency):
        if not isinstance(dependency, basestring):
            raise TypeError("Dependency must be a string representing a file name.")
        if dependency.strip() == "":
            raise ValueError("Dependency cannot be empty.")
        self.__include_dependencies.append(dependency)

    def add_source_file_dependency(self, number_of_components = 1):
        #Validate input values
        if number_of_components < 1:
            raise ValueError("You must use at least one path " \
                             "component in an include dependency.")

        #Make sure there is a source path
        if not self.__file_path:
            return

        #Build up the dependency path
        i = 1
        path, dependency = split(self.__file_path)
        while i < number_of_components:
            path, tail = split(path)
            if path == "":
                #Out of components!
                break
            dependency = join(tail, dependency)

        #Add the dependency
        self.add_include_dependency(dependency)

    @property
    def include_dependencies(self):
        return tuple(self.__include_dependencies)

_type_name_mappings = {
    cindex.TypeKind.CHAR_S: "char",
    cindex.TypeKind.VOID: "void",
    cindex.TypeKind.BOOL: "bool",
    cindex.TypeKind.CHAR_U: "unsigned char",
    cindex.TypeKind.UCHAR: "unsigned char",
    cindex.TypeKind.CHAR16: "char16_t",
    cindex.TypeKind.CHAR32: "char32_t",
    cindex.TypeKind.USHORT: "unsigned short",
    cindex.TypeKind.UINT: "unsigned int",
    cindex.TypeKind.ULONG: "unsigned long",
    cindex.TypeKind.ULONGLONG: "unsigned long long",
    cindex.TypeKind.UINT128: "uint128_t",
    cindex.TypeKind.CHAR_S: "char",
    cindex.TypeKind.SCHAR: "char",
    cindex.TypeKind.WCHAR: "wchat_t",
    cindex.TypeKind.SHORT: "short",
    cindex.TypeKind.INT: "int",
    cindex.TypeKind.LONG: "long",
    cindex.TypeKind.LONGLONG: "long long",
    cindex.TypeKind.INT128: "int128_t",
    cindex.TypeKind.FLOAT: "float",
    cindex.TypeKind.DOUBLE: "double",
    cindex.TypeKind.LONGDOUBLE: "long double",
    cindex.TypeKind.NULLPTR: "void *"
}

def _name_for_type(t):
    #Remove any typedefs
    canonical_kind = t.get_canonical().kind

    #See if we have the name mapped
    name = _type_name_mappings.get(canonical_kind, None)
    if name:
        return name

    #Otherwise just give the literal string
    return canonical_kind.name.lower()
    
def _function_declaration_finder(node, results, file_path):
    if node.kind == cindex.CursorKind.FUNCTION_DECL:
        #Grab the function information
        function_name = node.spelling
        return_type = _name_for_type(node.type.get_result())
        argument_types = tuple([_name_for_type(a_t)
                                for a_t 
                                in node.type.argument_types()])
        argument_names = tuple([n.spelling
                                for n 
                                in node.get_children()
                                if n.kind == cindex.CursorKind.PARM_DECL])

        #Grab the function location
        start = node.extent.start
        end = node.extent.end
        extent = (start.line, start.column, end.line, end.column)

        #Record the function
        results.append(CFunctionDeclaration(function_name, 
                                            return_type, 
                                            argument_types,
                                            argument_names,
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

    def __getitem__(self, key):
        for decl in self.__function_declarations:
            if decl.name == key:
                return decl

        #No matching declaration found
        return None
