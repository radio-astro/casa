#Example script running the VLA pipeline through task interfaces
#B. Kent, August 2013
#Updated Nov. 14, 2013

#MODIFY THESE TO THE PARENT DIRECTORY OF THE PIPELINE AND YOUR ASDM OR MS
basedir = '/lustre/naasc/bkent/'
#vis='13A-537.sb24066356.eb24324502.56514.05971091435'
vis = raw_input("Enter SDM filename: ")
vis=vis.rstrip('/')
if vis.endswith('.ms'):
    vis = vis[:-3]
vislist=[vis]



sys.path.insert (0, os.path.expandvars("$SCIPIPE_HEURISTICS"))

execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/h/cli/mytasks.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hif/cli/mytasks.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hifv/cli/mytasks.py"))

import pipeline
import os
import sys
import math
import numpy as np

#Initialize the context
h_init()
#h_init(loglevel='trace')


#Import data
hifv_importdata(vis=vislist)
h_save()

#Hanning smooth
#hifv_hanning()   #Remove this comment if you want to hanning smooth

#Deterministic flagging
hifv_flagdata()
h_save()

#Run setjy through the setmodel task
hifv_setmodel()
h_save()

#Gain curves, opacities, and requantizer gains
hifv_priorcals()
h_save()

#First determination of a reference antenna
hif_refant()
h_save()

#First test calibrations
hifv_testBPdcals()
h_save()
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/testBPdcals_plots.py"))

#Flag bad amp/phase deformatters
hifv_flagbaddef()
h_save()

#Unflag mode
hifv_uncalspw(delaycaltable='testdelay.k', bpcaltable='testBPcal.b')
h_save()

#Run rflag
hifv_checkflag()
h_save()

#Run test calibration
hifv_semiFinalBPdcals()
h_save()
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/semifinalBPdcals_plots1.py"))

#Run rflag
hifv_checkflag(checkflagmode='semi')
h_save()

#Run test calibration
hifv_semiFinalBPdcals()
h_save()
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/semifinalBPdcals_plots2.py"))

#Unflag mode
hifv_uncalspw(delaycaltable='delay.k', bpcaltable='BPcal.b')
h_save()

#Determine solints
hifv_solint()
h_save()

#Gaincal run
hifv_testgains()
h_save()
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/testgains_plots.py"))

#Flux density bootstrapping
hifv_fluxgains()
h_save()
hifv_fluxboot()
h_save()
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/fluxboot_plots.py"))

#Final gaincal runs
hifv_finalcals()
h_save()
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/finalcals_plots.py"))

#Final applycal
hifv_applycals()
h_save()

#Unflag mode
hifv_uncalspw(delaycaltable='finaldelay.k', bpcaltable='finalBPcal.b')
h_save()

#Flagging of targets
hifv_targetflag()
h_save()
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/recipes/vlaplots/targetflag_plots.py"))

#Run statwt
hifv_statwt()
h_save()

