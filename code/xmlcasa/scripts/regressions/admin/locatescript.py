import sys
import os
import string
import inspect
import shutil

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
        break
gl=sys._getframe(stacklevel).f_globals

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

def copydata(name, destdir):
    destdir = str(destdir)
    ok = False
    if not os.path.isdir(destdir):
        raise RuntimeError('destination directory (' + destdir + ') must exist...')
    for root, dirs, files in os.walk(gl['casa']['dirs']['data'] + "/regression"):
        if name in dirs:
            full_path = root + "/" + name
            shutil.copytree(full_path,destdir + "/" + name)
            ok = True
            break
        elif name in files:
            full_path = root + "/" + name
            shutil.copy(full_path,destdir)
            ok = True
            break

    if not ok:
        for root, dirs, files in os.walk(gl['casa']['dirs']['data']):
            if name in dirs:
                full_path = root + "/" + name
                shutil.copytree(full_path,destdir + "/" + name)
                ok = True
                break
            elif name in files:
                full_path = root + "/" + name
                shutil.copy(full_path,destdir)
                ok = True
                break

    if not ok:
        raise RuntimeError( 'failed to find "' + name + '" in ' + gl['casa']['dirs']['data'] )
