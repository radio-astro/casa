# alma-24ant-calibration-regression.py
# based on an ALMA QA2 Calibration Script

# CASA parallelization test version
# DP, 17 Oct 2012

import sys
import casadef

#print " *** init.py: QA2-relevant modules will be imported ***"
#sys.path.append('AIV/science/qa2')
#sys.path.append('AIV/science/analysis_scripts')

#from QA2_Packaging_module import *
#import analysisUtils as aU
#es = aU.stuffForScienceDataReduction()

#print aU.version()


mynumsubms = 18

# Calibration

thesteps = []
step_title = {0: 'Partitioning and listobs',
              1: 'A priori flagging',
              2: 'Generation and time averaging of the WVR cal table',
              3: 'Generation of the Tsys cal table',
              4: 'Generation of the antenna position cal table',
              5: 'Application of the WVR, Tsys and antpos cal tables',
              6: 'Split out science SPWs and time average',
              7: 'Listobs, clear pointing table, and save original flags',
              8: 'Initial flagging',
              9: 'Putting a model for the flux calibrator(s)',
              10: 'Save flags before bandpass cal',
              11: 'Bandpass calibration',
              12: 'Save flags before gain cal',
              13: 'Gain calibration',
              14: 'Save flags before applycal cal',
              15: 'Application of the bandpass and gain cal tables'}

try:
  print 'List of steps to be executed ...', mysteps
  thesteps = mysteps
except:
  print 'global variable mysteps not set.'
if (thesteps==[]):
  thesteps = range(0,len(step_title))
  print 'Executing all steps: ', thesteps

# The Python variable 'mysteps' will control which steps
# are executed when you start the script using
#   execfile('scriptForCalibration.py')
# e.g. setting
#   mysteps = [2,3,4]# before starting the script will make the script execute
# only steps 2, 3, and 4
# Setting mysteps = [] will make it execute all steps.

import re

#es = aU.stuffForScienceDataReduction() 


if re.search('^4.', casadef.casa_version) == None:
 sys.exit('ERROR: PLEASE USE CASA 4.x')

from simple_cluster import *


totaltime = 0
inittime = time.time()
ttime = inittime
steptime = []

def timing():
    global totaltime
    global inittime
    global ttime
    global steptime
    global step_title
    global mystep
    global thesteps
    thetime = time.time()
    dtime = thetime - ttime
    steptime.append(dtime)
    totaltime += dtime
    ttime = thetime
    casalog.origin('TIMING')
    casalog.post( 'Step '+str(mystep)+': '+step_title[mystep], 'WARN')
    casalog.post( 'Time now: '+str(ttime), 'WARN')
    casalog.post( 'Time used this step: '+str(dtime), 'WARN')
    casalog.post( 'Total time used so far: ' + str(totaltime), 'WARN')
    casalog.post( 'Step  Time used (s)     Fraction of total time (percent) [description]', 'WARN')
    for i in range(0, len(steptime)):
        casalog.post( '  '+str(thesteps[i])+'   '+str(steptime[i])+'  '+str(steptime[i]/totaltime*100.)
                      +' ['+step_title[thesteps[i]]+']', 'WARN')


# Using reference antenna = DV04

print "# A priori calibration"

# listobs
mystep = 0
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms*')

  if os.path.exists('clusterconf.txt'):
    if not os.path.exists('myclusterhome'):
      os.mkdir('myclusterhome')
    sl=simple_cluster()
    sl.init_cluster('clusterconf.txt', 'mycluster')
  
  partition(vis = 'uid___A002_X47bd4d_Xbd4.ms', outputvis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    numsubms=mynumsubms)

  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.listobs')
  listobs(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    listfile = 'hpc-uid___A002_X47bd4d_Xbd4.ms.listobs')
  
  timing()
  

# A priori flagging
mystep = 1
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
           mode = 'manual',
           autocorr = T,
           flagbackup = F)

  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
           mode = 'manual',
           antenna = 'DV14',
           flagbackup = F) 
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
           mode = 'manual',
           intent = '*POINTING*,*SIDEBAND_RATIO*,*ATMOSPHERE*',
           flagbackup = F)
  
  flagcmd(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    inpmode = 'table',
    action = 'plot')
  
  flagcmd(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    inpmode = 'table',
    action = 'apply')
  
  timing()

# Generation and time averaging of the WVR cal table
mystep = 2
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.wvr') 

  wvrgcal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.wvr',
    tie = ['targetx1,J0339-017'],
    statsource = 'targetx1')
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth') 
  
  smoothcal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    tablein = 'hpc-uid___A002_X47bd4d_Xbd4.ms.wvr',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth',
    smoothtype = 'mean',
    smoothtime = 6.048)
  
  
  #aU.plotWVRSolutions(caltable='hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth', spw='17', antenna='DV04',
  #  yrange=[-180,180],subplot=22, interactive=False,
  #  figfile='hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth.plots/hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth') 
  
  #Note: If you see wraps in these plots, try changing yrange or unwrap=True 
  #Note: If all plots look strange, it may be a bad WVR on the reference antenna.
  #      To check, you can set antenna='' to show all baselines.
  
  timing()

# Generation of the Tsys cal table
mystep = 3
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.tsys') 
  gencal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.tsys',
    caltype = 'tsys')
  
  #aU.plotbandpass(caltable='hpc-uid___A002_X47bd4d_Xbd4.ms.tsys', overlay='time', 
  #  xaxis='freq', yaxis='amp', subplot=22, buildpdf=False, interactive=False,
  #  showatm=True,pwv='auto',chanrange='5~122',showfdm=True, 
  #  field='', figfile='hpc-uid___A002_X47bd4d_Xbd4.ms.tsys.plots.overlayTime/hpc-uid___A002_X47bd4d_Xbd4.ms.tsys') 
  
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.tsys', msName='hpc-uid___A002_X47bd4d_Xbd4.ms', interactive=False) 

  timing()

# Generation of the antenna position cal table
mystep = 4
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  # Note: no baseline run found for antenna DV19.
  
  # Position for antenna DV18 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Note: no baseline run found for antenna DA45.
  
  # Position for antenna DA44 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DV13 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DV12 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DA41 is derived from baseline run made on 2012-03-25 04:42:17.
  
  # Position for antenna DV14 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DA43 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DA42 is derived from baseline run made on 2012-01-23 07:51:38.
  
  # Note: no baseline run found for antenna PM01.
  
  # Note: no baseline run found for antenna PM03.
  
  # Position for antenna PM02 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DV10 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Note: no baseline run found for antenna DV20.
  
  # Note: no baseline run found for antenna DV21.
  
  # Position for antenna DA46 is derived from baseline run made on 2012-03-25 04:42:17.
  
  # Position for antenna DV08 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DV09 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DV15 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Note: no baseline run found for antenna DV04.
  
  # Note: no baseline run found for antenna DV02.
  
  # Position for antenna DV03 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DV17 is derived from baseline run made on 2012-05-03 03:46:09.
  
  # Position for antenna DV16 is derived from baseline run made on 2012-05-03 03:46:09.
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.antpos') 
  gencal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.antpos',
    caltype = 'antpos',
    antenna = 'DA46,DV18,PM02,DV08,DV09,DA44,DV13,DV12,DA41,DV14,DA43,DA42,DV03,DV17,DV10,DV16,DV15',
    parameter = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])
  #  parameter = [0.000247000250965,-0.000508999451995,-0.000311999581754,0.000508686011,0.000919706893087,0.000473518503357,0.00018812692387,0.000660056575273,0.000469934652856,0.000428024806118,0.000192386926053,-7.13865255571e-05,0.000397703996595,0.000303499271338,-0.000252154165917,0.000465348935348,0.000344158009965,0.000228367319351,0.000584870703864,0.000952647723338,-0.000227043112602,-0.000133162034778,0.00023574780856,0.000212197651384,0.00060776527971,-0.000525096431375,-0.000386487226933,0.000293836351762,0.000269524465858,-8.05776686625e-05,0.000478540400055,0.000105861353228,3.76092994824e-05,0.000157838716115,0.000211078409778,-0.00037259206194,0.000227385910925,-0.000120083500757,-7.74828167994e-07,-7.6349849694e-05,0.000854968838704,1.63702564808e-05,0.000486001936163,7.96910315645e-05,0.000160790103968,6.20906828402e-05,0.000561427652214,0.000133509319652,0.00023328837383,0.000377916222577,-0.000436685084871])

  timing()


# Application of the WVR, Tsys and antpos cal tables
mystep = 5
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]
  
  from recipes.almahelpers import tsysspwmap
  tsysmap = tsysspwmap(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms', tsystable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.tsys')
  
  
  applycal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    field = '0',
    spw = '17,19,21,23',
    gaintable = ['hpc-uid___A002_X47bd4d_Xbd4.ms.tsys', 'hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth', 'hpc-uid___A002_X47bd4d_Xbd4.ms.antpos'],
    gainfield = ['0', '', ''],
    interp = 'linear,linear',
    spwmap = [tsysmap,[],[]],
    calwt = T,
    flagbackup = F)
  
  
  
  applycal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    field = '1',
    spw = '17,19,21,23',
    gaintable = ['hpc-uid___A002_X47bd4d_Xbd4.ms.tsys', 'hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth', 'hpc-uid___A002_X47bd4d_Xbd4.ms.antpos'],
    gainfield = ['1', '', ''],
    interp = 'linear,linear',
    spwmap = [tsysmap,[],[]],
    calwt = T,
    flagbackup = F)
  
  
  
  applycal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
    field = '2',
    spw = '17,19,21,23',
    gaintable = ['hpc-uid___A002_X47bd4d_Xbd4.ms.tsys', 'hpc-uid___A002_X47bd4d_Xbd4.ms.wvr.smooth', 'hpc-uid___A002_X47bd4d_Xbd4.ms.antpos'],
    gainfield = ['2', '', ''],
    interp = 'linear,linear',
    spwmap = [tsysmap,[],[]],
    calwt = T,
    flagbackup = F)
  
  #es.getCalWeightStats('hpc-uid___A002_X47bd4d_Xbd4.ms') 

  timing()


# Split out science SPWs and time average
mystep = 6
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split') 
  split(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms',
        outputvis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
        datacolumn = 'corrected',
        spw = '17,19,21,23',
        timebin = '6.048s',
        keepflags = T,
        keepmms = T)

  timing()


print "# Calibration"

# Listobs, clear pointing table, and save original flags
mystep = 7
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.listobs')
  listobs(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    listfile = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.listobs')
  
  flagmanager(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'save',
    versionname = 'Original')

  timing()


# Initial flagging
mystep = 8
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  # Flagging shadowed data
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'shadow',
    flagbackup = F)

  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DA46&DV15',
    scan = '17~28', spw='1',
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DV15&DV21', spw='1',
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DA41&DV08',
    scan = '4~8', spw='0',
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DV08&DV20',
    scan = '4~8', spw='0',
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DA42',
    spw='1:0~1461', 
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DA46',
    spw='1:0~1461', 
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DV08',
    spw='0:0~1475', 
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DV20',
    spw='0:0~1475', 
    flagbackup = F)
  
  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DV15',
    spw='1', 
    flagbackup = F)

  flagdata(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'manual',
    antenna = 'DV21',
    spw='1:0~1461', 
    flagbackup = F)

  timing()


# Putting a model for the flux calibrator(s)
mystep = 9
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  setjy(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    field = '1', # Callisto
    spw = '0,1,2,3',
    standard = 'Butler-JPL-Horizons 2010')
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.setjy.field*.png') 
  for i in ['1']:
    plotms(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
      xaxis = 'uvdist',
      yaxis = 'amp',
      ydatacolumn = 'model',
      field = i,
      spw = '0,1,2,3',
      avgchannel = '9999',
      coloraxis = 'spw',
      plotfile = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.setjy.field'+i+'.png')

  timing()


# Save flags before bandpass cal
mystep = 10
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  flagmanager(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'save',
    versionname = 'BeforeBandpassCalibration')
  
  timing()


# Bandpass calibration
mystep = 11
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.ap_pre_bandpass') 
  
  gaincal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.ap_pre_bandpass',
    field = '0', # J0339-017
    spw = '0:1536~2304,1:1536~2304,2:1536~2304,3:1536~2304',
    solint = 'int',
    refant = 'DV04',
    calmode = 'ap')
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.split.ap_pre_bandpass', msName='hpc-uid___A002_X47bd4d_Xbd4.ms.split', interactive=False) 
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass') 
  bandpass(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass',
    field = '0', # J0339-017
    solint = 'inf',
    combine = 'scan',
    refant = 'DV04',
    solnorm = T,
    bandtype = 'B',
    gaintable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.ap_pre_bandpass')
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch') 
  
  bandpass(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch',
    field = '0', # J0339-017
    solint = 'inf,20ch',
    combine = 'scan',
    refant = 'DV04',
    solnorm = T,
    bandtype = 'B',
    gaintable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.ap_pre_bandpass')
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass', msName='hpc-uid___A002_X47bd4d_Xbd4.ms.split', interactive=False) 
  
  timing()


# Save flags before gain cal
mystep = 12
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  flagmanager(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'save',
    versionname = 'BeforeGainCalibration')
  
  timing()


# Gain calibration
mystep = 13
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  # Note: the Solar system object used for flux calibration is highly resolved on some baselines.
  # Note: we will first determine the flux of the phase calibrator(s) on a subset of antennas.
  
  clearcal('hpc-uid___A002_X47bd4d_Xbd4.ms.split',field='J0339-017')
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_short_int') 
  gaincal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_short_int',
    field = '0~1', # J0339-017,Callisto
    selectdata = T,
    antenna = 'DA41,DA42,DV04,DV08,DV09,DV12,DV14,DV18,DV19,DV20&',
    solint = 'int',
    refant = 'DV04',
    gaintype = 'G',
    calmode = 'p',
    gaintable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch')
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_short_int', msName='hpc-uid___A002_X47bd4d_Xbd4.ms.split', interactive=False) 
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.ampli_short_inf') 
  gaincal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.ampli_short_inf',
    field = '0~1', # J0339-017,Callisto
    selectdata = T,
    antenna = 'DA41,DA42,DV04,DV08,DV09,DV12,DV14,DV18,DV19,DV20&',
    solint = 'inf',
    refant = 'DV04',
    gaintype = 'T',
    calmode = 'ap',
    gaintable = ['hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch', 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_short_int'])
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.split.ampli_short_inf', msName='hpc-uid___A002_X47bd4d_Xbd4.ms.split', interactive=False) 
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.flux_short_inf') 
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.fluxscale') 

  
  fluxscale(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.ampli_short_inf',
    fluxtable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.flux_short_inf',
    reference = '1') # Callisto
  
  f = open('hpc-uid___A002_X47bd4d_Xbd4.ms.split.fluxscale')
  fc = f.readlines()
  f.close()
  
  for phaseCalName in ['J0339-017']:
    for i in range(len(fc)):
      if fc[i].find('Flux density for '+phaseCalName) != -1 and re.search('in SpW=[0-9]+(?: \(ref SpW=[0-9]+\))? is: [0-9]+\.[0-9]+', fc[i]) != None:
        line = (re.search('in SpW=[0-9]+(?: \(ref SpW=[0-9]+\))? is: [0-9]+\.[0-9]+', fc[i])).group(0)
        spwId = (line.split('='))[1].split()[0]
        flux = float((line.split(':'))[1].split()[0])
        setjy(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
          field = phaseCalName.replace(';','*;').split(';')[0],
          spw = spwId,
          fluxdensity = [flux,0,0,0])
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_int') 
  gaincal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_int',
    field = '0~1', # J0339-017,Callisto
    solint = 'int',
    refant = 'DV04',
    gaintype = 'G',
    calmode = 'p',
    gaintable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch')
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_int', msName='hpc-uid___A002_X47bd4d_Xbd4.ms.split', interactive=False) 
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.flux_inf') 
  gaincal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.flux_inf',
    field = '0~1', # J0339-017,Callisto
    solint = 'inf',
    refant = 'DV04',
    gaintype = 'T',
    calmode = 'ap',
    gaintable = ['hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch', 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_int'])
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.split.flux_inf', msName='hpc-uid___A002_X47bd4d_Xbd4.ms.split', interactive=False) 
  
  os.system('rm -rf hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_inf') 
  gaincal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    caltable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_inf',
    field = '0~1', # J0339-017,Callisto
    solint = 'inf',
    refant = 'DV04',
    gaintype = 'G',
    calmode = 'p',
    gaintable = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch')
  
  #es.checkCalTable('hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_inf', msName='hpc-uid___A002_X47bd4d_Xbd4.ms.split', interactive=False) 
  
  timing()


# Save flags before applycal cal
mystep = 14
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  flagmanager(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    mode = 'save',
    versionname = 'BeforeApplycal')
  
  timing()


# Application of the bandpass and gain cal tables
mystep = 15
if(mystep in thesteps):
  casalog.post('Step '+str(mystep)+' '+step_title[mystep],'INFO')
  print 'Step ', mystep, step_title[mystep]

  for i in ['1']: # Callisto
    applycal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
      field = i,
      gaintable = ['hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch', 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_int', 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.flux_inf'],
      gainfield = ['', i, i],
      interp = 'linear,linear',
      calwt = F,
      flagbackup = F)
  
  applycal(vis = 'hpc-uid___A002_X47bd4d_Xbd4.ms.split',
    field = '0,2', # targetx1
    gaintable = ['hpc-uid___A002_X47bd4d_Xbd4.ms.split.bandpass_smooth20ch', 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.phase_inf', 'hpc-uid___A002_X47bd4d_Xbd4.ms.split.flux_inf'],
    gainfield = ['', '0', '0'], # J0339-017
    interp = 'linear,linear',
    calwt = F,
    flagbackup = F)
  
  timing()

