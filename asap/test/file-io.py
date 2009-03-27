#!/usr/bin/env python
from asap import *

import os
outdir = "./output"
if not os.path.exists(outdir):
    os.mkdir(outdir)

rcParams['verbose'] = 0

print "Test of file i/o"
rc('scantable', storage='disk')
data = scantable('data/tid-t002.rpf')
del data
rc('scantable',storage='memory')
data = scantable('data/tid-t002.rpf')

data.save('output/test.asap',overwrite=True)
data.save('output/test.ascii',format='ASCII',overwrite=True)
data.save('output/test.sdfits',format='SDFITS',overwrite=True)

data2 = scantable('output/test.sdfits')
data3 = scantable('output/test.asap')

print "File i/o test finished successfully"
