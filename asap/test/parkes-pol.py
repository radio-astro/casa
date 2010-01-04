#!/usr/bin/env python
from asap import *

import os
outdir = "./output"
if not os.path.exists(outdir):
    os.mkdir(outdir)

rcParams['verbose'] = 0
rcParams['plotter.ganged'] = 0

# Don't plot to the screen...
del plotter
plotter = asapplotter(False)

print "Test of Parkes polarimetry (P484)"

data_1665 = scantable('data/parkes-pol.rpf')
data_1665.rotate_linpolphase(-45)
data_1665.rotate_xyphase(-2)
data_1665.set_unit('km/s')
data_1665.set_freqframe('LSRK')

# Look at the first scan
selection = selector()
selection.set_scans(0)
data_1665.set_selection(selection)

d1_5 = data_1665.copy()
d1_7 = data_1665.copy()

d1_7.set_restfreqs([1667.3590],'MHz')

# Baseline both
msk = d1_5.create_mask([-30,-25],[-5,0])
d1_5.poly_baseline(msk,1)
msk = d1_7.create_mask([-30,-25],[-5,0])
d1_7.poly_baseline(msk,1)

# merge the two scans back together into a new scantable
plotscans = merge(d1_5,d1_7)
print plotscans.summary()
del d1_5,d1_7,data_1665
# Plot the results
plotter.set_mode('p','s')
plotter.set_layout(2,1)
plotter.set_range(-30,0)
selection.reset()
selection.set_polarisations(['I','Q', 'U', 'V'])
plotter.set_selection(selection)
selection.set_polarisations(['I','Plinear'])
plotter.set_selection(selection)
selection.set_polarisations(['RR','LL'])
plotter.set_selection(selection)
plotter.plot(plotscans)
plotter.save('output/parkes_rrll.png',dpi=80)

print "Parkes-Pol Test successful"
