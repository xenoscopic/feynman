#Common testing modules
import common

#Feynman modules
from feynman.common import validate_code_string, \
                           underscore_to_camel_case, \
                           c_string_literal_with_c_code

def test_code_string_validation():
    string_exception_pairs = (
        (None, TypeError),
        (5, TypeError),
        ("", ValueError),
        ("1", ValueError),
        ("a", None),
        ("good", None),
        ("lk 4g", ValueError),
        ("$1a", ValueError),
        ("  a4", ValueError)
    )

    for value, exception in string_exception_pairs:
        if exception == None:
            validate_code_string(value)
            continue

        thrown = False
        try:
            validate_code_string(value)
        except exception:
            thrown = True
        assert(thrown)

def test_underscore_to_camel_case():
    test_value_pairs = (
        ("_cern_physics", "CernPhysics"),
        ("cern_physics", "CernPhysics"),
        ("cern_physics_", "CernPhysics"),
        ("_cern__physics_", "Cern_Physics")
    )

    for test, value in test_value_pairs:
        assert(underscore_to_camel_case(test) == value)

def test_c_string_literal_with_c_code():
    test_value_pairs = (
        ("\t", '"\\t"'),
        ("\n", '"" \\\n""'),
        ("\"", '"\\""'),
        ("\\", '"\\\\"'),
        ("\r\n", '"" \\\n""'),
        ("\r", '"" \\\n""'),
        ("a", '"a"'),
        ("#include <test>", '"#include <test>"'),
        ('#include "test.h"', '"#include \\\"test.h\\\""'),
        ("5.2", '"5.2"')
    )

    for test, value in test_value_pairs:
        assert(c_string_literal_with_c_code(test) == value)
