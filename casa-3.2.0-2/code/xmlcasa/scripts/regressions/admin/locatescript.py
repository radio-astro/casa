import sys
import os
import string
import inspect

PYVER = str(sys.version_info[0]) + "." + str(sys.version_info[1])

SCRIPT_REPOS = os.environ["CASAPATH"].split()[0] + '/' + os.environ["CASAPATH"].split()[1] + '/python/' + PYVER

# To support casapy-test releases which have different structure
SCRIPT_REPOS2 = os.environ["CASAPATH"].split()[0]
SCRIPT_REPOS2 += "/lib64/python" + PYVER

SCRIPT_REPOS3 = os.environ["CASAPATH"].split()[0]
SCRIPT_REPOS3 += "/lib/python" + PYVER

SCRIPT_REPOS4 = os.environ["CASAPATH"].split()[0]
SCRIPT_REPOS4 += "/Resources/python"

def locatescript(lescript=''):
    a=os.popen("which casapy", "r")
    lepath=string.split(a.read(),"/bin")[0]
    if (os.path.exists(SCRIPT_REPOS+"/regressions/"+lescript)):
        lepath = SCRIPT_REPOS+"/regressions/"+lescript
    elif (os.path.exists(SCRIPT_REPOS2+"/regressions/"+lescript)):
        lepath = SCRIPT_REPOS2+"/regressions/"+lescript
    elif (os.path.exists(SCRIPT_REPOS3+"/regressions/"+lescript)):
        lepath = SCRIPT_REPOS3+"/regressions/"+lescript
    elif (os.path.exists(SCRIPT_REPOS4+"/regressions/"+lescript)):
        lepath = SCRIPT_REPOS4+"/regressions/"+lescript
    elif (os.path.exists(SCRIPT_REPOS+"/demos/"+lescript)):
        lepath = SCRIPT_REPOS+"/demos/"+lescript
    elif (os.path.exists(SCRIPT_REPOS+"/"+lescript)):
        lepath = SCRIPT_REPOS+"/"+lescript
    elif(os.path.exists(lepath+"/python/"+PYVER+"/"+lescript)):
        lepath=lepath+"/python/"+PYVER+"/"+lescript
    elif(os.path.exists(lepath+"/lib/python"+PYVER+"/"+lescript)):
        lepath=lepath+"/lib/python"+PYVER+"/"+lescript
    elif(os.path.exists('/usr/bin/casapyinfo')):         
        #locate the /usr/bin one
        a=os.popen("/usr/bin/casapyinfo --environ | grep CASAPATH", "r")
        x=a.read()
        lepath=string.split(string.split(x,'="')[1], " ")[0]+"/lib/python"+PYVER+"/"+lescript
    else:
        raise Exception, "Regression script %s not found "%(lescript)
    return lepath
