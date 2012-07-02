"""This module has various functions for environment specific setings.
"""
__all__ = ["is_casapy", "is_ipython", "setup_env", "get_revision",
           "is_asap_cli"]

import sys
import os
import shutil
import platform

def is_casapy():
    """Are we running inside casapy?"""
    try:
        from taskinit import casalog
        return True
    except ImportError:
        return False

def is_ipython():
    """Are we running inside IPython?"""
    return 'IPython' in sys.modules.keys()

def is_asap_cli():
    """Are we running inside asap ipython (but not casapy)"""
    return is_ipython() and not is_casapy()

def setup_env():
    """Set-up environment variables for casa and initialise ~/.asap on first
    use.
    """
    # Set up CASAPATH and first time use of asap i.e. ~/.asap/*
    plf = None
    if sys.platform == "linux2":
        if platform.architecture()[0] == '64bit':
            plf = 'linux_64b'
        else:
            plf = 'linux_gnu'
    elif sys.platform == 'darwin':
        plf = 'darwin'
    else:
        # Shouldn't happen - default to linux
        plf = 'linux'
    asapdata = os.path.split(__file__)[0]

    # Allow user defined data location
    if os.environ.has_key("ASAPDATA"):
        if os.path.exists(os.environ["ASAPDATA"]):
            asapdata = os.environ["ASAPDATA"]
    # use CASAPATH if defined and "data" dir present
    if not os.environ.has_key("CASAPATH") or \
            not os.path.exists(os.environ["CASAPATH"].split()[0]+"/data"):
        os.environ["CASAPATH"] = "%s %s somwhere" % ( asapdata, plf)
    # set up user space
    userdir = os.environ["HOME"]+"/.asap"
    if not os.path.exists(userdir):
        print 'First time ASAP use. Setting up ~/.asap'
        os.mkdir(userdir)
        if not is_casapy():
            shutil.copyfile(asapdata+"/data/ipy_user_conf.py",
                            userdir+"/ipy_user_conf.py")
        f = file(userdir+"/asapuserfuncs.py", "w")
        f.close()
        f = file(userdir+"/ipythonrc", "w")
        f.close()
    else:
        if not is_casapy():
            # upgrade to support later ipython versions
            if not os.path.exists(userdir+"/ipy_user_conf.py"):
               shutil.copyfile(asapdata+"/data/ipy_user_conf.py",
                               userdir+"/ipy_user_conf.py")

def get_revision():
    """Get the revision of the software. Only useful within casapy."""
    if not is_casapy:
        return ' unknown '
    casapath=os.environ["CASAPATH"].split()
    if os.path.isdir(casapath[0]+'/'+casapath[1]+'/python/2.5/asap'):
        # for casa developer environment (linux or darwin)
        revinfo=casapath[0]+'/'+casapath[1]+'/python/2.5/asap/svninfo.txt'
    else:
        # for end-user environments
        if casapath[1]=='darwin':
            revinfo=casapath[0]+'/Resources/python/asap/svninfo.txt'
        else:
            revinfo=casapath[0]+'/lib/python2.5/asap/svninfo.txt'
    if os.path.isfile(revinfo):
        f = file(revinfo)
        f.readline()
        revsionno=f.readline()
        f.close()
        return revsionno.rstrip()
    return ' unknown '
