import sys
import os
import string
import inspect
SCRIPT_REPOS= os.environ["CASAPATH"].split()[0]
SCRIPT_REPOS += "/code/xmlcasa/scripts"

# To support casapy-test releases which have different structure
SCRIPT_REPOS2 = os.environ["CASAPATH"].split()[0]
SCRIPT_REPOS2 += "/lib64/python2.5"

SCRIPT_REPOS3 = os.environ["CASAPATH"].split()[0]
SCRIPT_REPOS3 += "/lib/python2.5"

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
    elif(os.path.exists(lepath+"/python/2.5/"+lescript)):
        lepath=lepath+"/python/2.5/"+lescript
    elif(os.path.exists(lepath+"/lib/python2.5/"+lescript)):
        lepath=lepath+"/lib/python2.5/"+lescript
    elif(os.path.exists('/usr/bin/casapyinfo')):         
        #locate the /usr/bin one
        a=os.popen("/usr/bin/casapyinfo --environ | grep CASAPATH", "r")
        x=a.read()
        lepath=string.split(string.split(x,'="')[1], " ")[0]+"/lib/python2.5/"+lescript
    else:
        raise Exception, "Regression script %s not found "%(lescript)
    return lepath
