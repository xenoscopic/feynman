#System modules
from os.path import exists, isfile

#PyOpenCL modules
import pyopencl as cl

#Feynman modules
from .parsing import find_c_function_declarations

def platform_sort_key(platform):
    #Sort platforms based on number of devices
    return len(platform.get_devices())

def device_sort_key(device):
    #Sort based on device type, preferring:
    #Accelerator
    #GPU
    #CPU
    device_preference_map = {
        cl.device_type.CPU: 1,
        cl.device_type.GPU: 2,
        cl.device_type.ACCELERATOR: 3
    }
    return device_preference_map.get(device.type, 0)

class OpenClContext(object):
    def __init__(self):
        #Find the best platform
        platforms = sorted(cl.get_platforms(), key = platform_sort_key)
        if not platforms:
            raise RuntimeError("Unable to find any OpenCL platforms.")
        self.__platform = platforms[-1]

        #Find the best device
        devices = sorted(self.__platform.get_devices(), key = device_sort_key)
        if not devices:
            #Even though we're only checking one platform, we sort by platforms
            #with the most number of devices, so all other platforms have equal
            #or fewer devices.
            raise RuntimeError("Unable to find any OpenCL devices on this (or any) platform.")
        self.__device = devices[-1]

        #Create a context
        self.__context = cl.Context([self.__device])

        #Create a command queue
        self.__queue = cl.CommandQueue(self.__context, self.__device)

    @property
    def platform(self):
        return self.__platform

    @property
    def device(self):
        return self.__device

    @property
    def context(self):
        return self.__context

    @property
    def queue(self):
        return self.__queue

class OpenClSourceFile(object):
    def __init__(self, file_path):
        #Do basic file validation on file_path
        if not file_path:
            raise ValueError("The file_path argument must not be empty.")
        if not exists(file_path):
            raise ValueError("The file_path argument must point to a file which exists.")
        if not isfile(file_path):
            raise ValueError("The file_path argument must point to a file.")
        self.__file_path = file_path

        #Grab the function declarations
        self.__functions = find_c_function_declarations(file_path)

    @property
    def file_path(self):
        return self.__file_path

    @property
    def functions(self):
        return self.__functions
