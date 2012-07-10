import re

def validate_code_string(s):
    #Verify the type
    if not isinstance(s, basestring):
        raise TypeError("Specified value must be a string.")
    
    #Verify the content
    invalid_characters = re.compile("[^a-zA-Z0-9_]+")
    cleaned = invalid_characters.sub('', s)
    if cleaned != s:
        raise ValueError("The value must contain only " \
                         "alphanumeric characters.")
    if len(cleaned) == 0:
        raise ValueError("The value must contain at least "
                         "one letter.")
    if not cleaned[0].isalpha():
        raise ValueError("The value must start with a letter.")

def underscore_to_camel_case(s):
    #Remove edge underscores and capitalize
    s = s.strip("_")

    #Capitalize at underscore boundaries
    def camelcase(): 
        yield str.lower
        while True:
            yield str.capitalize

    c = camelcase()
    s = "".join(c.next()(x) if x else '_' for x in s.split("_"))

    #Capitalize the beginning.  Can't use capitalize here,
    #because it lower cases every other letter in the string.
    if len(s) > 0:
        s = s[:1].capitalize() + s[1:]
        
    return s

def c_string_literal_with_c_code(s):
    #Normalize line splits
    s = s.replace("\r\n", "\n").replace("\r", "\n")

    #Split up lines
    lines = s.split("\n")

    #Create the result
    return "\n".join(["\"%s\" \\" % l.encode("string_escape").replace("\"", "\\\"") for l in lines]).strip(" \\")

    