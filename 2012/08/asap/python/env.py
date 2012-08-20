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

def get_revision():
    """Get the revision of the software. Only useful within casapy."""
    if not is_casapy:
        return ' unknown '
    casapath=os.environ["CASAPATH"].split()
    versioninfo = sys.version_info
    pyversion = '%s.%s'%(versioninfo[0],versioninfo[1])
    if os.path.isdir(casapath[0]+'/'+casapath[1]+'/python/%s/asap'%(pyversion)):
        # for casa developer environment (linux or darwin)
        revinfo=casapath[0]+'/'+casapath[1]+'/python/%s/asap/svninfo.txt'%(pyversion)
    else:
        # for end-user environments
        if casapath[1]=='darwin':
            revinfo=casapath[0]+'/Resources/python/asap/svninfo.txt'
        else:
            revinfo=casapath[0]+'/lib/python%s/asap/svninfo.txt'%(pyversion)
    if os.path.isfile(revinfo):
        f = file(revinfo)
        f.readline()
        revsionno=f.readline()
        f.close()
        return revsionno.rstrip()
    return ' unknown '
