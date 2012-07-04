#PyOpenCL modules
import pyopencl as cl

def _platform_sort_key(platform):
    #Sort platforms based on number of devices
    return len(platform.get_devices())

def _device_sort_key(device):
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
        platforms = sorted(cl.get_platforms(), key = _platform_sort_key)
        if not platforms:
            raise RuntimeError("Unable to find any OpenCL platforms.")
        self.__platform = platforms[-1]

        #Find the best device
        devices = sorted(self.__platform.get_devices(), key = _device_sort_key)
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
