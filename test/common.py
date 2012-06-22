#System modules
from os.path import join, dirname, realpath

def resources_path():
    return join(dirname(realpath(__file__)), "resources")
