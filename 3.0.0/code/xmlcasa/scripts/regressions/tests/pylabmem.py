import sys
import os
import string
import inspect

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
gl=sys._getframe(stacklevel).f_globals
def run():
    pl=gl['pl']
    x=range(10000000)
    pl.ion()
    for k in range(100):
        pl.plot(x)
        pl.clf()
        pl.cla()
    return []

def data():
    return []
