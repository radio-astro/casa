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

print "Test of Tidbinbilla"

# Create the quotient spectra
data = scantable('data/tid-t002.rpf')
# IMPORTANT tid needs mode='time'
q = data.auto_quotient(mode='time')
# Set the restfreq for each IF
q.set_restfreqs(freqs= [23694.4700e6,23722.6336e6])
q.set_unit('km/s')
q.set_freqframe('LSRK')
# Align frequencies
q.freq_align()
# Recalculate the az/el
q.recalc_azel()
# Correct for gain curve and opacity
q.gain_el()
q.opacity(0.075)
# Average in time
av = q.average_time()
# Baseline
msk=av.create_mask([-70,-50],[40,60])
av.poly_baseline(mask=msk,order=1)
plotter.set_mode('i','s')
plotter.plot(av)
plotter.save('output/tid.png', dpi=80)

# Do some random processing, just to test these functions
av.smooth('gauss',5)
av.scale(1.05)
av.add(0.05)

print "Tid test finished successfully"
