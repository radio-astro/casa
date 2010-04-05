import sys
import os
import string
import inspect

gl=sys._getframe(len(inspect.stack())-1).f_globals
def run():
    pl=gl['pl']
    x=range(10000000)
    pl.ion()
    for k in range(10):
        pl.plot(x)
        pl.clf()
        pl.cla()
    return []

def data():
    return []
