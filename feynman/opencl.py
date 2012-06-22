#System modules
from os.path import exists, isfile

#PyOpenCL modules
import pyopencl as cl

#C parsing modules
import pycparser

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

    def get_platform(self):
        return self.__platform

    def set_platform(self, platform):
        raise AttributeError("The platform property is read-only.")

    def del_platform(self):
        raise AttributeError("The platform property is read-only.")

    def get_device(self):
        return self.__device

    def set_device(self, device):
        raise AttributeError("The device property is read-only.")

    def del_device(self):
        raise AttributeError("The device property is read-only.")

    def get_context(self):
        return self.__context

    def set_context(self, context):
        raise AttributeError("The context property is read-only.")

    def del_context(self):
        raise AttributeError("The context property is read-only.")

    def get_queue(self):
        return self.__queue

    def set_queue(self, queue):
        raise AttributeError("The queue property is read-only.")

    def del_queue(self):
        raise AttributeError("The queue property is read-only.")

    platform = property(get_platform, set_platform, del_platform)
    device = property(get_device, set_device, del_device)
    context = property(get_context, set_context, del_context)
    queue = property(get_queue, set_queue, del_queue)

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

        #Parse the source file
        parser = pycparser()

    def get_file_path(self):
        return self.__file_path

    def set_file_path(self, file_path):
        raise AttributeError("The file_path property is read-only.")

    def del_file_path(self):
        raise AttributeError("The file_path property is read-only.")

    file_path = property(get_file_path, set_file_path, del_file_path)

