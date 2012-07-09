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
