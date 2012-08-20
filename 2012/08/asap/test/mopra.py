#!/usr/bin/env python
from asap import *
rcParams['verbose'] = 0
print "Test using Mopra dual IF data..."
print "Reduction test..."
s = scantable("data/mopra_dualif.rpf")
q = s.auto_quotient()
del s
restfreqs = [110.201,86.243]     # 13CO-1/0, SiO the two IF
q.set_restfreqs(restfreqs,"GHz") # set the restfrequencies, as not in data
q.set_unit("km/s")               # set the unit to be used from now on
q.set_freqframe("LSRK")          # set frequency frame
q.auto_poly_baseline() # determine and subtract a poly baseline automatically
q.convert_flux() # K -> Jy
msk = q.create_mask([-70,20], [20,70]) # line free region - two windows
rms = q.stats("rms", msk)
med = q.stats("median", msk)

rcParams['plotter.gui'] = 0

def plotfit(scan, ifno):
    print "Fitter test for IF%d..." % ifno
    # select IF
    scan.set_selection(ifs=ifno)
    f = fitter()
    f.set_scan(scan)
    f.set_function(gauss=ifno+1) # fit gaussian(s)
    f.fit()
    f.plot(filename='output/moprafit_if%d.png' % ifno)
    fp = f.get_parameters()

for ifno in q.getifnos():
    plotfit(q, ifno)

print "Mopra Test successful"
