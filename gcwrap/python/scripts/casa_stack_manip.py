import sys
import string
import inspect

def stack_find(label="casa",level='stack') :
    a=inspect.stack()
    stacklevel=0
    if level == "stack":
        for k in range(len(a)):
            if a[k][1] == "<string>" or \
               a[k][1].startswith("<ipython-input-") or \
               string.find(a[k][1], 'ipython console') > 0 or \
               string.find(a[k][1],"casapy.py") > 0 or \
               string.find(a[k][1],"casa.py") > 0:
                stacklevel=k
                # jagonzal: Take the first level that matches the requirement
                break
    elif level == "root":
        for k in range(len(a)):
            if string.find(a[k][1],"start_casa.py") > 0:
                stacklevel=k
                # jagonzal: Take the first level that matches the requirement
                break
    else:
        raise RuntimeError("unknown stack level %s" % level)

    myf=sys._getframe(stacklevel).f_globals

    if myf.has_key(label) :
        return myf[label]
    else:
        return None
