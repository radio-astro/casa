""" User configuration file for IPython

This is a more flexible and safe way to configure ipython than *rc files
(ipythonrc, ipythonrc-pysh etc.)

This file is always imported on ipython startup. You can import the
ipython extensions you need here (see IPython/Extensions directory).

Feel free to edit this file to customize your ipython experience.

Note that as such this file does nothing, for backwards compatibility.
Consult e.g. file 'ipy_profile_sh.py' for an example of the things 
you can do here.

See http://ipython.scipy.org/moin/IpythonExtensionApi for detailed
description on what you could do here.
"""

# Most of your config files and extensions will probably start with this import

import IPython.ipapi
ip = IPython.ipapi.get()

# You probably want to uncomment this if you did %upgrade -nolegacy
import ipy_defaults 

import os   

def main():   
    o = ip.options
    o.autocall = 1
    o.system_verbose = 0
    
    o.prompt_in1 = r'ASAP>'
    #o.prompt_in2 = r'   .%n.:'
    o.prompt_in2 = r'.\D: '
    o.prompt_out = r'asap>:'

    #o.autoexec.append('%colors LightBG')
    #o.autoexec.append('%colors NoColor')
    o.autoexec.append('%colors Linux')
    o.autocall = 2
    o.pprint = 1
    o.confirm_exit = 0
    o.banner = 0
    o.xmode = 'Plain'
    o.readline_omit__names = 1
    import_all("asap")
    # asap banner
    execf("~/.asap/asapuserfuncs.py")
    ip.ex('print welcome()')

# some config helper functions you can use 
def import_all(modules):
    """ Usage: import_all("os sys") """ 
    for m in modules.split():
        ip.ex("from %s import *" % m)
        
def execf(fname):
    """ Execute a file in user namespace """
    ip.ex('execfile("%s")' % os.path.expanduser(fname))

main()

