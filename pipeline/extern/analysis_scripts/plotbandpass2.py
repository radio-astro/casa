#########################################################################
#
#  plotbandpass2.py
#
#  This is meant to be a generic task to display CASA bandpass solutions
#  with options to overlay them in various combinations.  It is meant to
#  work the 'new' (casa 3.4) calibration tables.  It relies on
#  the ValueMapping class written by Stuartt Corder in analysisUtils.py,
#  and is designed to be called from inside the analysisUtils namespace:
#      import analysisUtils as au
#      au.plotbandpass2()
#  or directly on its own:
#      import plotbandpass2 as p
#      p.plotbandpass2()
#
#  (not to be confused with plotbpass.py, which was written for a specific
#   purpose of analyzing ALMA bandpass stability)
#
#  Todd R. Hunter  April 2012
#

# Some regression commands to run after making major changes:
# ALMA data:
# au.plotbandpass('bandpass.bcal',showtsky=F,xaxis='freq',yaxis='amp',overlay='antenna',spw='',field='')
# au.plotbandpass('bandpass.bcal',showtsky=F,xaxis='freq',yaxis='both',phase=[-180,180],plotrange=[0,0,0,2])
# au.plotbandpass('bandpass.bcal',showtsky=F,xaxis='freq',yaxis='amp',overlay='antenna',spw='',field='',chanrange='1200~2000')
# au.plotbandpass('bandpass.bcal',showtsky=F,xaxis='chan',yaxis='amp',overlay='antenna',spw='',field='',plotrange=[1200,2000,0,0])
# au.plotbandpass('bandpass.bcal',showtsky=F,xaxis='freq',yaxis='phase',overlay='antenna',spw='',field='')
# au.plotbandpass('bandpass.bcal',showtsky=F,xaxis='freq',yaxis='phase',overlay='antenna',spw='',field='',chanrange='1200~1800')
# au.plotbandpass('bandpass.bcal',overlay='antenna',yaxis='amp',field='0~1,4',xaxis='freq',showtsky=T)
# au.plotbandpass('bandpass_b_skipspw19high.bcal',yaxis='amp',field='0',xaxis='freq',caltable2='bandpass_bpoly_skipspw19high.bcal',showpoints=True,spw=0,figfile='bpoly_overlay')
# au.plotbandpass('bandpass_b_skipspw19high.bcal',yaxis='phase',field='0',xaxis='freq',caltable2='bandpass_bpoly_skipspw19high.bcal',showpoints=True,spw=0,figfile='bpoly_overlay',caltable3='bandpass_bpoly_skipspw19high.bcal')
# au.plotbandpass('bandpass_b_skipspw19high.bcal',yaxis='both',field='0',xaxis='freq',caltable2='bandpass_bpoly_skipspw19high.bcal',showpoints=True,spw=0,figfile='bpoly_overlay',caltable3='bandpass_bpoly_skipspw19high.bcal')
# au.plotbandpass('X3c1.tsys.fdm',overlay='antenna',yaxis='amp',field='1',xaxis='chan',figfile='tsys.png',showtsky=T)
# au.plotbandpass('X3c1.tsys.fdm',overlay='antenna',yaxis='amp',field='1',xaxis='chan',figfile='tsys.png',poln='y')
# au.plotbandpass('X3c1.tsys',overlay='antenna',yaxis='amp',field='0~1,4',xaxis='chan',figfile='tsys.png')
# au.plotbandpass('X3c1.tsys',overlay='time',yaxis='amp',field='2',xaxis='chan')
# au.plotbandpass('X3c1.tsys',overlay='',yaxis='amp',field='2',xaxis='freq',chanrange='45~65')
# au.plotbandpass('bandpass_bpoly_skipspw19high.bcal')
# au.plotbandpass('bandpass.bcal',caltable2='bandpass.bcal_smooth',xaxis='freq')
# au.plotbandpass('bandpass.bcal',caltable2='bandpass.bcal_smooth',yaxis='phase', xaxis='freq')
# au.plotbandpass('bandpass.bcal',caltable2='bandpass.bcal_smooth',xaxis='freq')
# au.plotbandpass('bandpass.bcal',caltable2='bandpass.bcal_smooth',xaxis='freq',chanrange='1000~3000')
# au.plotbandpass('bandpass.bcal',caltable2='bandpass.bcal_smooth',xaxis='freq',showtsky=T)
# au.plotbandpass('bandpass.bcal',caltable2='bandpass.bcal_smooth',xaxis='freq',poln='x',showflagged=True)
# au.plotbandpass('bandpass.bcal',caltable2='bandpass.bcal_smooth',xaxis='freq',poln='x',showflagged=True, showtsky=T)
# au.plotbandpass(caltable='X3c1.tsys.fdm',caltable2='X3c1.tsys',    yaxis='amp',xaxis='freq',figfile='tsys.png')
# au.plotbandpass(caltable='X3c1.tsys.fdm',caltable2='X3c1.tsys',    yaxis='amp',xaxis='freq',figfile='tsys.png', showtsky=T)
# au.plotbandpass(caltable='X3c1.tsys',    caltable2='X3c1.tsys.fdm',yaxis='amp',xaxis='freq',figfile='tsys.png')
# au.plotbandpass(caltable='X3c1.tsys',    caltable2='X3c1.tsys.fdm',yaxis='amp',xaxis='freq',figfile='tsys.png', showtsky=T)
# au.plotbandpass(caltable='X3c1.tsys.fdm',caltable2='X3c1.tsys',    yaxis='both',xaxis='freq',figfile='tsys.png',chanrange='1000~3000')
# au.plotbandpass(caltable='X3c1.tsys.fdm',caltable2='X3c1.tsys',    yaxis='both',xaxis='freq',figfile='tsys.png',chanrange='1000~3000',showtsky=T)
# au.plotbandpass(caltable='X3c1.tsys',    caltable2='X3c1.tsys.fdm',yaxis='both',xaxis='freq',figfile='tsys.png')
# au.plotbandpass(caltable='X3c1.tsys',    caltable2='X3c1.tsys.fdm',yaxis='both',xaxis='freq',figfile='tsys.png', showtsky=T)
# au.plotbandpass(caltable='X3c1.tsys.fdm',caltable2='X3c1.tsys',    yaxis='both',xaxis='freq',figfile='tsys.png',zoom='intersect')
# au.plotbandpass(caltable='X3c1.tsys',    caltable2='X3c1.tsys.fdm',yaxis='both',xaxis='freq',figfile='tsys.png',zoom='intersect')
# au.plotbandpass(caltable='X3c1.tsys.fdm',caltable2='X3c1.tsys',yaxis='amp',xaxis='freq',figfile='tsys.png',poln='XX')
# au.plotbandpass(caltable='X3c1.tsys.fdm',caltable2='X3c1.tsys',yaxis='amp',field='1',xaxis='freq',figfile='tsys.png',poln='YY',zoom='intersect')
#
# tests for multi-field solution overlay: in SciVer/TWHya
# au.plotbandpass('band7multi_a6p7_titan.bcal',caltable2='band7multi_b.bcal',debug=F,xaxis='freq',yaxis='amp',chanrange='')
# au.plotbandpass(caltable='band7multi_b.bcal',caltable3='band7multi_bpoly_a6p7_titan.bcal',caltable2='band7multi_bpoly.bcal',xaxis='freq',yaxis='both')
#
# EVLA dual-pol data:
#  cd /export/lustre/thunter/evla/10C-186/K_BnA_cont/K_BnA_cont_multi.ms
#  vm = au.ValueMapping('K_BnA_cont_multi.ms')
#  au.plotbandpass('bandpasspcal.bcal',poln='',yaxis='both',vm=vm)
#  au.plotbandpass('bandpasspcal.bcal',poln='',yaxis='both',overlay='antenna',vm=vm)
#  au.plotbandpass('bandpasspcal.bcal',poln='',yaxis='amp',overlay='antenna',vm=vm, chanrange='0~30',xaxis='freq')
#  au.plotbandpass('bandpasspcal.bcal',poln='',yaxis='both',showatm=T,vm=vm)
#  au.plotbandpass('bandpasspcal.bcal',poln='LL',yaxis='both',vm=vm)
#
# EVLA single-pol data:
#  cd /export/lustre/thunter/evla/AB1346/g19.36
#  vm2 = au.ValueMapping('g19.36_I_3sD_multi.ms')
#  au.plotbandpass('bandpass.bcal',caltable2='bandpass_bpoly.bcal',yaxis='both',xaxis='freq',vm=vm2)
#
PLOTBANDPASS_REVISION_STRING = "$Id: plotbandpass2.py,v 1.1.2.1 2012/09/06 15:03:47 swilliam Exp $" 
import pylab as pb
import math, os, sys, re
import time as timeUtilities
import numpy as np
import analysisUtils as au
from taskinit import *
from matplotlib.ticker import MultipleLocator, FormatStrFormatter, ScalarFormatter

TOP_MARGIN  = 0.25   # Used if showatm=T or showtksy=T
BOTTOM_MARGIN = 0.25 # Used if showfdm=T

# This is a color sequence found online which has distinguishable colors
overlayColors = [
      [0.00,  0.00,  0.00],
      [0.00,  0.00,  1.00],
      [0.00,  0.50,  0.00],
      [1.00,  0.00,  0.00],
      [0.00,  0.75,  0.75],
#      [0.75,  0.00,  0.75], # magenta, same as atmcolor
      [0.25,  0.25,  0.25],
      [0.75,  0.25,  0.25],
      [0.95,  0.95,  0.00],
      [0.25,  0.25,  0.75],
#      [0.75,  0.75,  0.75],  this color is invisible for some reason
      [0.00,  1.00,  0.00],
      [0.76,  0.57,  0.17],
      [0.54,  0.63,  0.22],
      [0.34,  0.57,  0.92],
      [1.00,  0.10,  0.60],
#      [0.88,  0.75,  0.73],  invisible
      [0.10,  0.49,  0.47],
      [0.66,  0.34,  0.65],
      [0.99,  0.41,  0.23]]
overlayColors += overlayColors + overlayColors  # 17*3 = 51 total colors

# Enumeration to keep track of plot pages
PAGE_ANT = 0
PAGE_SPW = 1
PAGE_TIME = 2
PAGE_AP = 3

# Used to parse command line arguments
myValidCharacterList = ['~', ',', ' ', '*',] + [str(m) for m in range(10)]
myValidCharacterListWithBang = ['~', ',', ' ', '*', '!',] + [str(m) for m in range(10)]
LARGE_POSITIVE = +1e20
LARGE_NEGATIVE = -1e20
maxAntennaNamesAcrossTheTop = 17
maxTimesAcrossTheTop = 17
antennaVerticalSpacing = 0.016
antennaHorizontalSpacing = 0.05
timeHorizontalSpacing = 0.05
xstartTitle = 0.07
ystartTitle = 0.95
xstartPolLabel = 0.05
ystartOverlayLegend = 0.933
opaqueSky = 270. # Kelvin, used for scaling TebbSky

developerEmail = "thunter@nrao.edu"

#class Polarization:
    # taken from Stokes.h in casa
#    (Undefined, I,Q,U,V,RR,RL,LR,LL,XX,XY,YX,YY) = range(13)  

def refTypeToString(rtype):
    rtypes = ['REST','LSRK','LSRD','BARY','GEO','TOPO','GALACTO','LGROUP','CMB']
    return(rtypes[rtype])

def corrTypeToString(ptype):
    ptypes = ['Undefined','I','Q','U','V','RR','RL','LR','LL','XX','XY','YX','YY']
    mystring = ptypes[ptype]
#    print "mystring = %s" % (mystring)
    return(mystring)
    
def buildAntString(antID,msFound,msAnt):
      if (msFound):
          antstring = msAnt[antID]
      else:
          antstring = str(antID)
      return(antstring)
      
      
def makeplot(figfile,msFound,msAnt,overlayAntennas,pages,pagectr,density,
             interactive,antennasToPlot,spwsToPlot,overlayTimes,
             locationCalledFrom):
  if (type(figfile) == str):
    if (figfile.find('/')>=0):
      directories = figfile.split('/')
      directory = ''
      for d in range(len(directories)-1):
          directory += directories[d] + '/'
      if (os.path.exists(directory)==False):
          print "Making directory = ", directory
          os.system("mkdir -p %s" % directory)
  debug = False
  if (debug):
      print "makeplot(%d): pagectr=%d, len(pages)=%d, len(spwsToPlot)=%d, pages=" % (locationCalledFrom,
                                                            pagectr, len(pages),len(spwsToPlot)), pages
  ispw = spwsToPlot[pages[pagectr][PAGE_SPW]]
  t = pages[pagectr][PAGE_TIME] + 1
  antstring = buildAntString(antennasToPlot[pages[pagectr][PAGE_ANT]], msFound, msAnt)
  figfile = figfile.split('.png')[0]
  if (msFound):
    if (overlayAntennas):
      plotfilename = figfile+'.spw'+str(ispw)+'.t'+str(t)
    elif (overlayTimes):
      plotfilename = figfile+'.'+antstring+'.spw'+str(ispw)
    else:
      plotfilename = figfile+'.'+antstring+'.spw'+str(ispw)+'.t'+str(t)
  else:
    if (overlayAntennas):
      plotfilename = figfile+'.spw'+str(ispw)+'.t'+str(t)
    elif (overlayTimes):
      plotfilename = figfile+'.ant'+antstring+'.spw'+str(ispw)
    else:
      plotfilename = figfile+'.ant'+antstring+'.spw'+str(ispw)+'.t'+str(t)
  plotfilename += '.png'
  if (interactive == False or 1==1):
    print "Building %s" % (plotfilename)
  pb.savefig(plotfilename, format='png', dpi=density)
  return(plotfilename)

def utdatestring(mjdsec):
    (mjd, dateTimeString) = au.mjdSecondsToMJDandUT(mjdsec)
    tokens = dateTimeString.split()
    return(tokens[0])

def utstring(mjdsec, xframeStart=110):
    (mjd, dateTimeString) = au.mjdSecondsToMJDandUT(mjdsec)
    tokens = dateTimeString.split()
    hoursMinutes = tokens[1][0:len(tokens[1])-3]
    hoursMinutesSeconds = tokens[1][0:len(tokens[1])]
    if (xframeStart == 110):  # 2011-01-01 UT 00:00
        return(tokens[0]+' '+tokens[2]+' '+hoursMinutes)
    elif (xframeStart == 3):
        return(hoursMinutesSeconds)
    else:  # 00:00
        return(hoursMinutes)
    
def openBpolyFile(caltable):
   tb.open(caltable)
   desc = tb.getdesc()
   if ('POLY_MODE' in desc):
      polyMode = tb.getcol('POLY_MODE')
      print "This is a BPOLY solution = %s" % (polyMode[0])
      polyType = tb.getcol('POLY_TYPE')
      scaleFactor = tb.getcol('SCALE_FACTOR')
      antenna1 = tb.getcol('ANTENNA1')
      times = tb.getcol('TIME')
      cal_desc_id = tb.getcol('CAL_DESC_ID')
      nRows = len(polyType)
      for pType in polyType:
          if (pType != 'CHEBYSHEV'):
              print "I do not recognized polynomial type = %s" % (pType)
              return
      # Here we assume that all spws have been solved with the same mode
      uniqueTimesBP = np.unique(tb.getcol('TIME'))
      nUniqueTimesBP = len(uniqueTimesBP)
      print "There are %d unique times in the BPOLY solution:" % (nUniqueTimesBP)
      if (nUniqueTimesBP == 2):
          print "differing by %g seconds" % (uniqueTimesBP[1]-uniqueTimesBP[0])
      mystring = ''
      for u in uniqueTimesBP:
          mystring += '%.12f,' % u
      print mystring
      nPolyAmp = tb.getcol('N_POLY_AMP')
      nPolyPhase = tb.getcol('N_POLY_PHASE')
      frequencyLimits = tb.getcol('VALID_DOMAIN')
      increments = 0.001*(frequencyLimits[1,:]-frequencyLimits[0,:])
      frequenciesGHz = []
      for i in range(len(increments)):
         freqs = (1e-9)*np.arange(frequencyLimits[0,i],frequencyLimits[1,i],increments[i])
         frequenciesGHz.append(freqs)
      polynomialAmplitude = []
      polynomialPhase = []
      for i in range(len(polyMode)):
        polynomialAmplitude.append([1])
        polynomialPhase.append([0])
        if (polyMode[i] == 'A&P' or polyMode[i] == 'A'):
          polynomialAmplitude[i]  = tb.getcell('POLY_COEFF_AMP',i)[0][0][0]
        if (polyMode[i] == 'A&P' or polyMode[i] == 'P'):
          polynomialPhase[i] = tb.getcell('POLY_COEFF_PHASE',i)[0][0][0]
      bpoly = True
      tb.close()
      tb.open(caltable+'/CAL_DESC')
      nSpws = len(tb.getcol('NUM_SPW'))
      tb.close()
      nPolarizations = len(polynomialAmplitude[0]) / nPolyAmp[0]
      print "(3)Set nPolarizations = %d" % nPolarizations
      
      # This value is overridden by the new function doPolarizations in ValueMapping.
      # print "Inferring %d polarizations from size of polynomial array" % (nPolarizations)
      return([polyMode, polyType, nPolyAmp, nPolyPhase, scaleFactor, nRows, nSpws, nUniqueTimesBP,
              uniqueTimesBP, nPolarizations, frequencyLimits, increments, frequenciesGHz,
              polynomialPhase, polynomialAmplitude, times, antenna1, cal_desc_id])
   else:
      tb.close()
      return([])
   # end of openBpolyFile()

def displayTimesArray(uniqueTimesPerFieldPerSpw):
  """
  Displays an array of MJD second timestamps as UT timestamps
  """
  legendString = ''
  for s in uniqueTimesPerFieldPerSpw:
    legendString += "["
    for f in s:
        legendString += "["
        for t in f:
            legendString += "%s" % utstring(t,2)
            if (t != f[-1]):
                legendString += ", "
        legendString += "]"
        if (f != s[-1]):
            legendString += ', '
    legendString += "], "
    if (s != uniqueTimesPerFieldPerSpw[-1]):
        legendString += ', '
  print legendString          

def checkPolsToPlot(polsToPlot, corr_type_string):
  firstFailure = 0
  for pol in polsToPlot:
        if ((pol in corr_type_string) == False):
              print "Polarization product %s is not in the ms" % (pol)
              firstFailure += 1
              if (pol in ['XX','YY']):
                    polsToPlot = ['RR','LL']
              else:
                    polsToPlot = ['XX','YY']
              break
  if (firstFailure>0):
     print "Looking for instead: ", polsToPlot
     for pol in polsToPlot:
        if ((pol in corr_type_string) == False):
              print "Polarization product %s is not in the ms" % (pol)
              firstFailure += 1
              if (pol in ['XX']):
                    polsToPlot = ['YY']
              elif (pol in ['YY']):
                    polsToPlot = ['XX']
              elif (pol in ['RR']):
                    polsToPlot = ['LL']
              elif (pol in ['LL']):
                    polsToPlot = ['RR']
              break
  if (firstFailure > 1):
     print "Looking for instead: ", polsToPlot
     for pol in polsToPlot:
        if ((pol in corr_type_string) == False):
              print "Polarization product %s is not in the ms" % (pol)
              return([])
  return(polsToPlot)

def getCorrType(msName):
      tb.open(msName+'/POLARIZATION')
      i = 0
      corr_type = tb.getcell('CORR_TYPE',i)
#      print "len(corr_type) = %d" % (len(corr_type))
      while (len(corr_type) > 2):
         i += 1;
         try:
             corr_type = tb.getcell('CORR_TYPE',i)
#             print "len(corr_type) = %d" % (len(corr_type))
         except:
             # We have reached the final row, and have only seen 4-pol data.
             print "This is a 4-polarization dataset."
             break
      tb.close()
      corr_type_string = []
      if (len(corr_type) == 4):
          if (corr_type[0] in [5,6,7,8]):
              corr_type = [5,8]
          elif (corr_type[0] in [9,10,11,12]):
              corr_type = [9,12]
          else:
              print "Unsupported polarization types = ", corr_type
              return(corr_type, corr_type_string)
      # This overrides the len(gain_table) because it can have length=2 even when only 1 pol present
      nPolarizations = len(corr_type)
#      print "(2)Set nPolarizations = %d" % nPolarizations
      for ct in corr_type:
            corr_type_string.append(corrTypeToString(ct))
      print "corr_types = ", corr_type,  " = ", corr_type_string
      return(corr_type, corr_type_string, nPolarizations)

def writeArgument(f,name,arg):
    if (type(arg) == str):
        s = "%-18s = '%s'" % (name,arg)
        t = "%s='%s'" % (name,arg)
    else:
        s = "%-18s = %s" % (name,str(arg))
        t = "%s=%s" % (name,arg)
    f.write(s+'\n')
    return(t)

def plotbandpass2(caltable='', antenna='', field='', spw='', yaxis='amp',
                 xaxis='chan', figfile='', plotrange=[0,0,0,0], help=False,
                 caltable2='', overlay='', showflagged=False, timeranges='',
                 buildpdf=False, caltable3='', markersize=3, density=108,
                 interactive=True, showpoints='auto', showlines='auto',
                 subplot='22', zoom='', poln='', showatm=False, pwv='auto',
                 gs='gs', convert='convert', chanrange='',
                 solutionTimeThresholdSeconds=60.0, debug=False, vm='',
                 phase='',  ms='', showtsky=False, showfdm=False,showatmfield='',
                 lo1=None, showimage=False, showatmPoints=False):
  """
  This is a tool to plot bandpass solutions faster than plotcal.  It is designed
  to work for the new cal table format introduced in casa 3.4.  The source code
  is in plotbandpass2.py. If it detects a new cal table, it will automatically
  call plotbandpass, which was developed for casa 3.3 cal tables. For more
  detailed help, run au.plotbandpass2(help=True) or see examples at:
  http://casaguides.nrao.edu/index.php?title=Plotbandpass
  -- Todd Hunter
  """
  print "%s" % (PLOTBANDPASS_REVISION_STRING)
  DEBUG = debug
  if (help):
      print "Usage: plotbandpass2(caltable='', antenna='', field='', spw='', yaxis='amp',"
      print "   xaxis='chan', figfile='', plotrange=[0,0,0,0], help=False, caltable2=''," 
      print "   overlay='', showflagged=False, timeranges='', buildpdf=False, caltable3='',"
      print "   markersize=3, density=108, interactive=True, showpoints='auto',"
      print "   showlines='auto', subplot='22', zoom='', poln='', showatm=False, pwv='auto',"
      print "   gs='gs', convert='convert', chanrange='', debug=False, vm='',"
      print "   solutionTimeThresholdSeconds=60.0, phase='', ms='', showtsky=False,"
      print "   showfdm=False, showatmfield='', lo1=None, showimage=False,"
      print "   showatmPoints=False)"
      print " antenna: must be ID (int or string or list), or a single antenna name or list"
      print " atm: overlay the atmospheric transmission curve for the weather and elevation"
      print " buildpdf: True/False, if True and figfile is set, assemble pngs into a pdf"
      print " caltable: a bandpass table, of type B or BPOLY"
      print " caltable2: a second cal table, of type BPOLY or B, to overlay on a B table"
      print " caltable3: a third cal table, of type BPOLY, to overlay on the first two"
      print " chanrange: set xrange ('5~100') over which to autoscale y-axis for xaxis='freq'"
      print " convert: full path for convert command (in case it's not found)"
      print " density: dpi to use in creating PNGs and PDFs (default=108)"
      print " field: must be an ID, source name, or list thereof; can use trailing *: 'J*'"
      print " figfile: the base_name of the png files to save: base_name.antX.spwY.png"
      print " gs: full path for ghostscript command (in case it's not found)"
      print " help: print this message"
      print " interactive: if False, then figfile will run to completion automatically"
      print " lo1: specify the LO1 setting (in GHz) for the observation (used if showimage=T)"
      print " overlay: 'antenna' or 'time', make 1 plot with different items in colors"
      print " showflagged:  show the values of data, even if flagged"
      print " markersize: size of points (default=3)"
      print " ms: name of the parent ms, in case it does not match the string in the caltable"
      print " phase: the y-axis limits to use for phase plots when yaxis='both'"
      print " plotrange: define axis limits: [x0,x1,y0,y1] where 0,0 means auto"
      print " poln: polarizations to plot (e.g. 'XX','YY','RR','LL' or '' for both)"
      print " pwv: define the pwv to use for the showatm option: 'auto' or value in mm"
      print " showatm: compute and overlay the atmospheric transmission curve"
      print " showatmfield: for overlay='time', use first observation of this fieldID or name"
      print " showatmPoints: draw atmospheric curve with points instead of a line"
      print " showfdm: when showing TDM spws with xaxis='freq', draw locations of FDM spws"
      print " showimage: also show the atmospheric curve for the image sideband (in black)"
      print " showtsky: compute and overlay the sky temperature curve instead of transmission"
      print " showlines: draw lines connecting the data (default=T for amp, F for phase)"
      print " showpoints: draw points for the data (default=F for amp, T for phase)"
      print " solutionTimeThresholdSeconds: consider 2 solutions simultaneous if within this interval (default=60)"
      print " spw: must be single ID or list or range (e.g. 0~4, not the original ID)"
      print " subplot: 11,22,32 or 42 for RowsxColumns (default=22), any 3rd digit is ignored"
      print " timeranges: show only these timeranges, the first timerange being 1"
      print " vm: the result from ValueMapping('my.ms'), or as returned from a previous call to plotbandpass"
      print " xaxis: 'chan' or 'freq'"
      print " yaxis: 'amp', 'tsys', 'phase', or 'both' amp&phase == 'ap'; append 'db' for dB"
      print " zoom: 'intersect' will zoom to overlap region of caltable with caltable2"
      return(vm)
  mytimestamp = timeUtilities.time()
  debugSloppyMatch = False
  doneTimeOverlay = True
  missingCalWVRErrorPrinted = False

  # Write a .last file
  cmd = 'plotbandpass2'
  lastfile = open('%s.last'%cmd,'w')
  lastfile.write('taskname           = "%s"\n'%cmd)
  cmd += '(' + writeArgument(lastfile, "caltable", caltable)
  cmd += ',' + writeArgument(lastfile, "antenna" , antenna)
  cmd += ',' + writeArgument(lastfile, "field" , field)
  cmd += ',' + writeArgument(lastfile, "spw" , spw)
  cmd += ',' + writeArgument(lastfile, "yaxis", yaxis)
  cmd += ',' + writeArgument(lastfile, "xaxis", xaxis)
  cmd += ',' + writeArgument(lastfile, "figfile", figfile)
  cmd += ',' + writeArgument(lastfile, "plotrange" , plotrange)
  cmd += ',' + writeArgument(lastfile, "help", help)
  cmd += ',' + writeArgument(lastfile, "caltable2", caltable2)
  cmd += ',' + writeArgument(lastfile, "overlay", overlay)
  cmd += ',' + writeArgument(lastfile, "showflagged", showflagged)
  cmd += ',' + writeArgument(lastfile, "timeranges", timeranges)
  cmd += ',' + writeArgument(lastfile, "buildpdf", buildpdf)
  cmd += ',' + writeArgument(lastfile, "caltable3", caltable3)
  cmd += ',' + writeArgument(lastfile, "markersize", markersize)
  cmd += ',' + writeArgument(lastfile, "density", density)
  cmd += ',' + writeArgument(lastfile, "interactive", interactive)
  cmd += ',' + writeArgument(lastfile, "showpoints", showpoints)
  cmd += ',' + writeArgument(lastfile, "showlines", showlines)
  cmd += ',' + writeArgument(lastfile, "subplot", subplot)
  cmd += ',' + writeArgument(lastfile, "zoom", zoom)
  cmd += ',' + writeArgument(lastfile, "poln", poln)
  cmd += ',' + writeArgument(lastfile, "showatm", showatm)
  cmd += ',' + writeArgument(lastfile, "showatmfield", showatmfield)
  cmd += ',' + writeArgument(lastfile, "pwv", pwv)
  cmd += ',' + writeArgument(lastfile, "gs", gs)
  cmd += ',' + writeArgument(lastfile, "convert", convert)
  cmd += ',' + writeArgument(lastfile, "chanrange", chanrange)
  cmd += ',' + writeArgument(lastfile, "solutionTimeThresholdSeconds", solutionTimeThresholdSeconds)
  cmd += ',' + writeArgument(lastfile, "debug", debug)
  cmd += ',' + writeArgument(lastfile, "vm", vm)
  cmd += ',' + writeArgument(lastfile, "phase", phase)
  cmd += ',' + writeArgument(lastfile, "ms", ms)
  cmd += ',' + writeArgument(lastfile, "lo1", lo1)
  cmd += ',' + writeArgument(lastfile, "showimage", showimage)
  cmd += ',' + writeArgument(lastfile, "showtsky", showtsky)
  cmd += ',' + writeArgument(lastfile, "showatmPoints", showatmPoints)
  cmd += ',' + writeArgument(lastfile, "showfdm", showfdm) + ')'
  lastfile.write('#%s\n'%(cmd))
  lastfile.close()
  
  if (showimage == False):
      LO1 = lo1 = None
  if (showatm and showtsky):
      print "You have selected both showatm and showtsky!  Defaulting to showatm=True only."
      showtsky = False
  if (showatm==False and showtsky==False and showatmfield!=''):
      print "Defaulting to showatm=True because showatmfield was specified."
      showatm = True
  if (overlay.find('time') < 0 and showatmfield != ''):
      print "The showatmfield only has meaning for overlay='time'."
      return(vm)
  
  if (plotrange=='' or plotrange==[]):
      plotrange = [0,0,0,0]
  if (type(plotrange) != list):
      print "plotrange must be an array:  [0,1,-180,180]"
      return(vm)
  if (len(plotrange) < 4):
      print "plotrange must be an array:  [0,1,-180,180]"
      return(vm)
  if (phase != ''):
      if (type(phase) != list):
          print "phase must be either '' or 2 values: [x,y]"
          return(vm)
      if (len(phase) != 2):
          print "phase must be either '' or 2 values: [x,y]"
          return(vm)

  if (buildpdf and figfile==''):
      print "With buildPDF=True, you must specify figfile='yourFileName' (.png will be appended if necessary)."
      return(vm)

  if (interactive==False and figfile==''):
      print "With interactive=False, you must specify figfile='yourFileName' (.png will be appended if necessary)."
      return(vm)

  pxl = 0 # polarization number to use for setting xlimits if plotrange=[0,0...]
  if (type(chanrange) != str):
      if (type(chanrange) != list):
          print "Chanrange must be a string or list:  '8~120' or [8,120]"
          return(vm)
      elif (len(chanrange) != 2):
          print "Chanrange must be a string or list:  '8~120' or [8,120]"
          return(vm)
      elif ((type(chanrange[0]) != int) or (type(chanrange[1]) != int)):
          print "Chanrange list members must be integers, not ", type(chanrange[0]), type(chanrange[1])
          return
  elif (len(chanrange) < 1):
      chanrange = [0,0]
  else:
      if (chanrange.find('~')<0):
          print "Invalid chanrange, no tilde found"
          return(vm)
      tokens = chanrange.split('~')
      if (len(tokens) < 2):
          print "Invalid chanrange, too few tokens"
          return(vm)
      if (xaxis.find('chan')>=0):
              print "The chanrange parameter is only valid for xaxis='freq', and only if the plotrange is [0,0,0,0]."
              return(vm)
      try:
          chanrange = [int(tokens[0]),int(tokens[1])]
          if (DEBUG):
              print "Using chanrange = ", chanrange
      except:
          print "Invalid chanrange, not integers"
          return(vm)
  if (chanrange[0] < 0):
      print "Invalid chanrange, cannot be negative"
      return(vm)
  if ((chanrange[0] != 0 or chanrange[1] != 0) and (plotrange[0] != 0 or plotrange[1] != 0 or plotrange[2] != 0 or plotrange[3] != 0)):
      print "If chanrange is specified, then plotrange must be all zeros."
      return(vm)
      
  if (pwv==''):
      pwv = 1.0
  if (type(poln) != list):
        poln = poln.upper()
  if (poln == 'X'):
        poln = 'XX'
  if (poln == 'Y'):
        poln = 'YY'
  if (poln == 'X,Y' or poln=='Y,X'):
        poln = 'XX,YY'
  if (poln == 'R'):
        poln = 'RR'
  if (poln == 'L'):
        poln = 'LL'
  if (poln == 'R,L' or poln=='L,R'):
        poln = 'RR,LL'

  # Parse the polarizations to plot from the command line
  # Prior to opening the .ms (later), we cannot tell which products are actually present
  useAllPols = False
  if (poln == ''):
        useAllPols = True
        polsToPlot = ['XX','YY']  # assume ALMA initially
  elif (type(poln) == list):
        polsToPlot = poln.split(',')
  else:
        if ((poln in ['','RR','RL','LR','LL','XX','XY','YX','YY','RR,LL','XX,YY']) == False):
              print "Unrecognized polarization option = ", poln
              return(vm)
        if (poln.find(',')>0):
            polsToPlot = poln.split(',')
        else:
            polsToPlot = [poln]
      
        
  if ((overlay in ['antenna', 'time', '']) == False):
     print "Unrecognized option for overlay: only 'antenna' and 'time' are supported, not both."
     return(vm)
     
  allowedFrames = [11,22,32,42]
  if (int(subplot) > 100):
      # This will accept 111, 221, 321, 421, etc.
      subplot /= 10
  if ((int(subplot) in allowedFrames)==False):
    print "Subplot choice (rows x columns) must be one of ", allowedFrames
    print "(with an optional trailing digit that is ignored)."
    return(vm)

  if (yaxis.find('both')<0 and yaxis.find('ap')<0 and yaxis.find('tsys')<0 and
      yaxis.find('amp')<0 and yaxis.find('phase')<0):
      print "Invalid yaxis.  Must be 'amp', 'tsys', 'phase' or 'both'."
      return(vm)

  if (yaxis.find('tsys')>=0):
      yaxis = 'amp'

  if (xaxis.find('chan')<0 and xaxis.find('freq')<0):
      print "Invalid xaxis.  Must be 'chan' or 'freq'."
      return(vm)

  if (showatm and showtsky):
      print "showatm=True and showtsky=True are mutually exclusive options"
      return(vm)

  if (showfdm and xaxis.find('freq')<0):
      print "The option showfdm=True requires xaxis='freq'."
      return(vm)

  # Plotting settings
  minPhaseRange = 0.2
  plotfiles = []
  if (int(subplot) == 11):
    mysize = '10'
    titlesize = '10'
  elif (int(subplot) == 22 or int(subplot) == 32):
    mysize = '8'
    titlesize = '8'
  else:
    mysize = '7'
    titlesize = '8'
  if (type(subplot) == str):
      subplot = int(subplot)
  validSubplots = [11,22,32,42]
  if (subplot in validSubplots == False):
      print "Invalid subplot = %d.  Valid options are: " % (subplot), validSubplots
      return(vm)
  xframeStart = int(subplot)*10  # i.e. 110 or 220 or 420
  firstFrame = xframeStart + 1
  lastFrame = xframeStart + (subplot/10)*(subplot%10)
  bottomRowFrames = [111,223,224,325,326,427,428]  # try to make this more general
  leftColumnFrames = [111,221,223,321,323,325,421,423,425,427]
  rightColumnFrames = [111,222,224,322,324,326,422,424,426,428]
  subplotCols = subplot % 10
  subplotRows = subplot/10
  ystartPolLabel = 1.0-0.04*subplotRows
  if (subplotCols == 1):
      fstringLimit = 40 # character length of multi-field overlay title string
  elif (subplotCols == 2):
      fstringLimit = 12 # character length of multi-field overlay title string
  
#  print "xframeStart = %d" % (xframeStart)
  xframe = xframeStart
#  print "Using markersize = ", markersize
  xcolor = 'b'
  ycolor = 'g'
  pcolor = ['b','g']
  x2color = 'k'
  y2color = 'c'
  p2color = ['k','c']
  x3color = 'm'
  y3color = 'r'
  p3color = ['m','r']
  if (showpoints == 'auto'):
      if (showlines == 'auto'):
          ampmarkstyle = '-'
          phasemarkstyle = '.'
          if (len(polsToPlot) == 1):
                ampmarkstyle2 = '-'
          else:
                ampmarkstyle2 = '--'
          phasemarkstyle2 = 'o'
      elif (showlines == False):
          ampmarkstyle = '.'
          ampmarkstyle2 = 'o'
          phasemarkstyle = '.'
          phasemarkstyle2 = 'o'
      else:
          ampmarkstyle = '-'
          phasemarkstyle = '-'
          if (len(polsToPlot) == 1):
                ampmarkstyle2 = '-'
                phasemarkstyle2 = '-'
          else:
                ampmarkstyle2 = '--'
                phasemarkstyle2 = '--'
  elif (showpoints == True):
      if (showlines == 'auto'):
          ampmarkstyle = '.-'
          phasemarkstyle = '.'
          if (len(polsToPlot) == 1):
                ampmarkstyle2 = 'o-'
          else:
                ampmarkstyle2 = 'o--'
          phasemarkstyle2 = 'o'
      elif (showlines == False):
          ampmarkstyle = '.'
          ampmarkstyle2 = 'o'
          phasemarkstyle = '.'
          phasemarkstyle2 = 'o'
      else:
          ampmarkstyle = '.-'
          phasemarkstyle = '.-'
          if (len(polsToPlot) == 1):
                ampmarkstyle2 = 'o-'
                phasemarkstyle2 = 'o-'
          else:
                ampmarkstyle2 = 'o--'
                phasemarkstyle2 = 'o--'
  else:  # showpoints == False
      if (showlines == False):
          print 'You must have either showpoints or showlines set True or auto, assuming showlines=T'
      ampmarkstyle = '-'
      phasemarkstyle = '-'
      if (len(polsToPlot) == 1):
            ampmarkstyle2 = '-'
            phasemarkstyle2 = '-'
      else:
            ampmarkstyle2 = '--'
            phasemarkstyle2 = '--'

  ampmarkstyles = [ampmarkstyle,ampmarkstyle2]
  phasemarkstyles = [phasemarkstyle,phasemarkstyle2]
  # bpoly solutions should always be shown as lines, not dots or dots+lines
  bpolymarkstyle = '-'

  amplitudeWithPhase = (yaxis.find('both')>=0 or yaxis.find('ap')>=0)
  if (amplitudeWithPhase):
      myhspace = 0.30
      if (overlay.find('antenna')>=0 or overlay.find('time')>=0):
          print "Option 'overlay' is incompatible with yaxis='both'.  Pick either amp or phase."
          return(vm)
  else:
      myhspace = 0.30
  if (subplot/10 > 2):
      myhspace = 0.4
  if (subplot/10 > 3):
      myhspace = 0.6
  mywspace = 0.25
  
  # Now open the Bandpass solution table
  try:
      tb.open(caltable)
  except:
      print "Could not open the caltable = %s" % (caltable)
      return(vm)
  names = tb.colnames()
  if ('SPECTRAL_WINDOW_ID' not in names):
       print "This appears to be an old-format cal table from casa 3.3 or earlier.  Using plotbandpass."
       return(3.3)
  casalog.post(cmd)
  ant = tb.getcol('ANTENNA1')
  fields = tb.getcol('FIELD_ID')
  if (DEBUG):
      print "FIELD_ID column = ", fields
  validFields = False
  for f in fields:
      if (f != -1):
          validFields = True
  if (validFields == False):
      print "The field_id is -1 (invalid) for all rows of this caltable."
      print "Did you remember to run assignFieldAndScanToSolution()?"
      return(vm)
  try:
      flags = tb.getcol('FLAG')
  except:
      print "No Flag column found. Are you sure this is a bandpass solution file, or is it the .ms?"
      print "If it is a solution file, does it contain solutions for both TDM and FDM spws?"
      return(vm)
  times = tb.getcol('TIME')
  intervals = tb.getcol('INTERVAL')
  cal_desc_id = tb.getcol('SPECTRAL_WINDOW_ID')
  uniqueSpwsInCalTable = np.unique(cal_desc_id)
  ParType = tb.getkeyword('ParType')    # string = 'Complex'
  msName = tb.getkeyword('MSName')      
  VisCal = tb.getkeyword('VisCal')      # string = 'B TSYS'
  PolBasis = tb.getkeyword('PolBasis')  # string = 'LINEAR'
  spectralWindowTable = tb.getkeyword('SPECTRAL_WINDOW').split()[1]
  tb.close()
  
  tb.open(spectralWindowTable)
  chanFreqGHz = []
  originalSpws = range(len(tb.getcol('MEAS_FREQ_REF')))
  for i in originalSpws:
      # They array shapes can vary.
      chanFreqGHz.append(1e-9 * tb.getcell('CHAN_FREQ',i))
  chanFreqGHz2 = chanFreqGHz
  
  originalSpw = originalSpws  # may need to do a global replace of this <----------------------------------
  uniqueTimes = sloppyUnique(np.unique(times), solutionTimeThresholdSeconds)
  nUniqueTimes = len(uniqueTimes)
  if (nUniqueTimes == 1):
      print "Found solutions with %d unique time within a threshold of %d seconds." % (nUniqueTimes,solutionTimeThresholdSeconds)
  else:
      print "Found solutions with %d unique times (within a threshold of %d seconds)." % (nUniqueTimes,solutionTimeThresholdSeconds)
  mystring = ''
  if (debug):
     for u in uniqueTimes:
         mystring += '%.6f, ' % (u)
     print mystring
  uniqueAntennaIds = np.unique(ant)
  uniqueFields = np.unique(fields)
  nFields = len(uniqueFields)
  spwlist = []
  uniqueTimesPerFieldPerSpw = []
  for s in uniqueSpwsInCalTable:
        uniqueTimesPerField = []
        for f in uniqueFields:
            timelist = []
            for row in range(len(fields)):
                if (fields[row] == f and cal_desc_id[row] == s):
                    if (sloppyMatch(times[row], timelist, solutionTimeThresholdSeconds) == False):
                        timelist.append(times[row])
                        spwlist.append(cal_desc_id)
            uniqueTimesPerField.append(timelist)
        uniqueTimesPerFieldPerSpw.append(uniqueTimesPerField)
  tb.close()

  displayTimesArray([[uniqueTimes]])

  # Parse the spws to plot from the command line
  if (spw==''):
     spwsToPlot = uniqueSpwsInCalTable
  else:
     if (type(spw) == str):
           if (spw.find('!')>=0):
                 print "The ! modifier is not (yet) supported"
                 return(vm)
           tokens = spw.split(',')
           spwsToPlot = []
           for token in tokens:
                 if (len(token) > 0):
                       if (token.find('*')>=0):
                             spwsToPlot = uniqueSpwsInCalTable
                             break
                       elif (token.find('~')>0):
                             (start,finish) = token.split('~')
                             spwsToPlot +=  range(int(start),int(finish)+1)
                       else:
                             spwsToPlot.append(int(token))
     elif (type(spw) == list):
         spwsToPlot = np.sort(spw)
     else:
         spwsToPlot = [spw]
         
  if (len(uniqueSpwsInCalTable) > 1):
      print "%d spws in the solution = " % (len(uniqueSpwsInCalTable)), uniqueSpwsInCalTable
  else:
      print "%d spw in the solution = " % (len(uniqueSpwsInCalTable)), uniqueSpwsInCalTable
  for myspw in spwsToPlot:
      if (myspw not in uniqueSpwsInCalTable):
          print "spw %d is not in the solution" % (myspw)
          return
      
  if (len(uniqueFields) > 1):
      print "%d fields in the solution = " % (len(uniqueFields)), uniqueFields
  else:
      print "%d field in the solution = " % (len(uniqueFields)), uniqueFields
  
  # Figure out which kind of Bandpass solution this is.
  bOverlay = False  # Am I trying to overlay a second B-type solution?
  if (os.path.exists(caltable) == False):
        print "Caltable does not exist = %s" % (caltable)
        return(vm)
  try:
      ([polyMode, polyType, nPolyAmp, nPolyPhase, scaleFactor, nRows, nSpws, nUniqueTimesBP, uniqueTimesBP,
        nPolarizations, frequencyLimits, increments, frequenciesGHz, polynomialPhase,
        polynomialAmplitude, timesBP, antennasBP, cal_desc_idBP]) = openBpolyFile(caltable)
      bpoly = True
      bpolyOverlay = bpolyOverlay2 = False
      if (xaxis.find('chan') >= 0):
          print "Sorry, but BPOLY solutions cannot be plotted with xaxis='chan'. Proceeding with xaxis='freq'."
          xaxis = 'freq'
      if (chanrange[0] != 0 or chanrange[1] != 0):
          print "The chanrange parameter only applies if the first caltable is a B solution, not a BPOLY."
          return(vm)
      if (len(caltable2) > 0):
          try:
              # figure out if the next file is a BPOLY or another B solution to pick the proper error message.
              ([polyMode, polyType, nPolyAmp, nPolyPhase, scaleFactor, nRows, nSpws, nUniqueTimesBP, uniqueTimesBP,
                nPolarizations, frequencyLimits, increments, frequenciesGHz, polynomialPhase,
                polynomialAmplitude, timesBP, antennasBP, cal_desc_idBP]) = openBpolyFile(caltable2)
              print "Sorry, but you cannot overlay two BPOLY solutions (unless caltable is a B solution and caltable2 and 3 are BPOLYs)."
          except:
              print "Sorry, but for overlays, caltable must be a B solution, whlie caltable2 and 3 can be either type."
          return(vm)
  except:
      print "This is a %s solution." % (VisCal)
      bpoly = bpolyOverlay = bpolyOverlay2 = False

      # Now check if there is a second file to overlay
      if (len(caltable2) > 0):
        if (os.path.exists(caltable2) == False):
              print "Caltable2 does not exist = %s" % (caltable2)
              return(vm)
        try:
          # figure out if the next file is a BPOLY or another B solution
          ([polyMode, polyType, nPolyAmp, nPolyPhase, scaleFactor, nRows, nSpws, nUniqueTimesBP, uniqueTimesBP,
            nPolarizations, frequencyLimits, increments, frequenciesGHz, polynomialPhase,
            polynomialAmplitude, timesBP, antennasBP, cal_desc_idBP]) = openBpolyFile(caltable2)
          bpolyOverlay = True
          print "Overlay the BPOLY solution"
          if (xaxis.find('chan')>=0):
              print "Sorry, but overlap of BPOLY is currently possible only with xaxis='freq'"
              return(vm)
          if (len(caltable3) > 0):
             if (os.path.exists(caltable3) == False):
                   print "Caltable3 does not exist = %s" % (caltable3)
                   return(vm)
             bpolyOverlay2 = True
             print "Overlay the second BPOLY solution"
             ([polyMode2, polyType2, nPolyAmp2, nPolyPhase2, scaleFactor2, nRows2, nSpws2,
               nUniqueTimesBP2, uniqueTimesBP2,
               nPolarizations2, frequencyLimits2, increments2, frequenciesGHz2, polynomialPhase2,
               polynomialAmplitude2, timesBP2, antennasBP2, cal_desc_idBP2]) = openBpolyFile(caltable3)
        except:
            # this is another B solution
            print "Overlay another %s solution" % (VisCal)
            bOverlay = True
            if (xaxis.find('freq')<0):
                  print "Currently, you must use xaxis='freq' to overlay two B solutions."
                  return(vm)
            if (len(caltable3) > 0):
                  print "You cannot overlay caltable3 because caltable2 is a B solution."
                  return(vm)
      elif (len(caltable3) > 0):
          print "You cannot have a caltable3 argument without a caltable2 argument."
          return(vm)
          
  if (overlay.find('antenna')>=0):
      overlayAntennas = True
      if (bpoly == True):
            print "The overlay of times or antennas is not supported with BPOLY solutions"
            return(vm)
      if (len(caltable2)>0):
            print "The overlay of times or antennas not supported when overlaying a B or BPOLY solution"
            return(vm)
      print "Will overlay solutions from different antennas"
  else:
      overlayAntennas = False

  if (overlay.find('time')>=0):
      overlayTimes = True
      if (bpoly == True):
            print "The overlay of times or antennas is not supported with BPOLY solutions"
            return(vm)
      if (len(caltable2)>0):
            print "The overlay of times or antennas not supported when overlaying a B or BPOLY solution"
            return(vm)
      print "Will overlay solutions from different times"
  else:
      overlayTimes = False
      
  if (bOverlay):        
        # Now open the Bandpass solution table
        try:
              tb.open(caltable2)
        except:
              print "Could not open the second caltable = %s" % (caltable2)
              return(vm)
        names = tb.colnames()
        if ('SPECTRAL_WINDOW_ID' not in names):
            if ('SNR' not in names):
                print "This does not appear to be a cal table."
                return(vm)
            else:
                print "This appears to be an old-format cal table from casa 3.3 or earlier.  You cannot mix formats with overlay."
                return(vm)
        cal_desc_id2 = tb.getcol('SPECTRAL_WINDOW_ID')

        uniqueSpwsInCalTable2 = np.unique(cal_desc_id2)
        msName2 = tb.getkeyword('MSName')      
        ParType2 = tb.getkeyword('ParType')    # string = 'Complex'
        VisCal2 = tb.getkeyword('VisCal')      # string = 'B TSYS'
        PolBasis2 = tb.getkeyword('PolBasis')  # string = 'LINEAR'
        spectralWindowTable2 = tb.getkeyword('SPECTRAL_WINDOW').split()[1]
        ant2 = tb.getcol('ANTENNA1')
        fields2 = tb.getcol('FIELD_ID')
        try:
            flags2 = tb.getcol('FLAG')
        except:
            print "No Flag column found. Are you sure this is a bandpass solution file, or is it the .ms?"
            print "If it is a solution file, does it contain solutions for both TDM and FDM spws?"
            return(vm)
        times2 = tb.getcol('TIME')
        uniqueTimes2 = sloppyUnique(np.unique(times2), solutionTimeThresholdSeconds)
        nUniqueTimes2 = len(uniqueTimes2)
#        print "Found %d solutions in time: MJD seconds = " % (nUniqueTimes2), uniqueTimes2
        spacing = ''
        for i in range(1,nUniqueTimes2):
            spacing += '%.0f, ' % (np.abs(uniqueTimes2[i]-uniqueTimes2[i-1]))
        print "Found %d solutions in time, spaced by seconds: " % (nUniqueTimes2), spacing
        displayTimesArray([[uniqueTimes2]])
        uniqueAntennaIds2 = np.unique(ant2)
        uniqueFields2 = np.unique(fields2)
        nFields2 = len(uniqueFields2)
        tb.close()
        tb.open(spectralWindowTable2)
        chanFreqGHz2 = []
        originalSpws2 = range(len(tb.getcol('MEAS_FREQ_REF')))
        for i in originalSpws2:
            # They array shapes can vary.
            chanFreqGHz2.append(1e-9 * tb.getcell('CHAN_FREQ',i))
            
        originalSpws2 = range(len(tb.getcol('MEAS_FREQ_REF')))
        originalSpw2 = originalSpws2  # may want to do a global replace of this <----------------------------------
        print "(boverlay) original unique spws in the second dataset = ", np.unique(originalSpw2)

        uniqueTimesPerFieldPerSpw2 = []
        for s in uniqueSpwsInCalTable2:
          uniqueTimesPerField2 = []
          for f in uniqueFields2:
              timelist2 = []
              for row in range(len(fields2)):
                    if (fields2[row] == f and cal_desc_id2[row] == s):
                          if (sloppyMatch(times2[row], timelist2, solutionTimeThresholdSeconds) == False):
                                timelist2.append(times2[row])
              uniqueTimesPerField2.append(timelist2)
          uniqueTimesPerFieldPerSpw2.append(uniqueTimesPerField2)
        print "uniqueTimesPerFieldPerSpw2 = " #, uniqueTimesPerFieldPerSpw2
        displayTimesArray(uniqueTimesPerFieldPerSpw2)
          
        if (len(uniqueSpwsInCalTable2) > 1):
              print "%d spws in the second solution = " % (len(uniqueSpwsInCalTable2)), uniqueSpwsInCalTable2
        else:
              print "%d spw in the second solution = " % (len(uniqueSpwsInCalTable2)), uniqueSpwsInCalTable2
        if (len(uniqueFields2) > 1):
              print "%d fields in the solution = " % (len(uniqueFields2)), uniqueFields2
        else:
              print "%d field in the solution = " % (len(uniqueFields2)), uniqueFields2

  # Parse the timeranges field from the command line
  if (type(timeranges) == str):
         # a list of antenna numbers was given
         tokens = timeranges.split(',')
         timerangeList = []
         for token in tokens:
             if (len(token) > 0):
                 if (token.find('~')>0):
                     (start,finish) = token.split('~')
                     timerangeList +=  range(int(start),int(finish)+1)
                 else:
                     timerangeList.append(int(token))
         if (len(timerangeList) < 1):
             # then a blank list was specified
             timerangeList = list(1+np.array(range(len(uniqueTimes))))
  elif (type(timeranges) == list):
      # it's already a list of integers
      timerangeList = timeranges
  else:
      # It's a single, integer entry
      timerangeList = [timeranges]

  timerangeListOneBased = timerangeList
  # convert from 1-based to 0-based counting
  if (0 in timerangeList):
      print "Sorry, timeranges start at 1, not 0."
      return(vm)
  timerangeList = list(np.array(timerangeList) - 1)
  timerangeListTimes = np.array(uniqueTimes)[timerangeList]
  timerangeListTimesString = ''  
  for t in timerangeListTimes:
      timerangeListTimesString += utstring(t,4) + ' '
  print "UT times to plot: ", timerangeListTimesString
  print "Corresponding time IDs (1-based):", timerangeListOneBased 

  # Now open the associated ms tables via ValueMapping
  msAnt = []
  if (vm == ''):
    if (os.path.exists(msName)):
      try:
          print "Running ValueMapping on %s..." % (msName)
          print "(This can take awhile, try the vm option next time: vm=au.plotbandpass(..)"
          print "                         then, on subsequent calls:    au.plotbandpass(..,vm=vm)"
          vm = au.ValueMapping(msName)
#          print "Done ValueMapping"
          donetime = timeUtilities.time()
          print "%.1f sec elapsed" % (donetime-mytimestamp)
          mytimestamp = timeUtilities.time()
          msAnt = vm.antennaNamesForAntennaIds
          msFields = vm.fieldNamesForFieldIds
          print "Available antennas = ", msAnt
      except:
          print "1)Could not open the associated measurement set tables (%s). Will not translate antenna names or frequencies." % (msName)
    else:
      if (ms==''):
          print "Could not find the associated measurement set (%s). Will not translate antenna names or frequencies." % (msName)
      else:
          # Use the ms name passed in from the command line
          msName = ms
#          print "************* 2) Set msName to ", msName
          try:
              print "Running ValueMapping on %s..." % (msName)
              print "(This can take a minute, try using the vm option next time)"
              vm = au.ValueMapping(msName)
              donetime = timeUtilities.time()
              print "%.1f sec elapsed" % (donetime-mytimestamp)
              mytimestamp = timeUtilities.time()
              msAnt = vm.antennaNamesForAntennaIds
              msFields = vm.fieldNamesForFieldIds
              print "Available antennas = ", msAnt
          except:
              print "1) Could not open the associated measurement set tables (%s). Will not translate antenna names or frequencies." % (msName)
  else:
      if (msName.find(vm.getInputMs()) < 0):
          print "WARNING:  There is a mismatch between the ms name in the ValueMapping"
          print "structure provided and the one associated with this bandpass solution:"
          print "   %s vs. %s" % (vm.getInputMs(), msName)
#          return(vm)
      print "Using the ValueMapping result provided as an argument"
      msAnt = vm.antennaNamesForAntennaIds
      msFields = vm.fieldNamesForFieldIds
      
  msFound =  False
  if (len(msAnt) > 0):
        msFound = True
        print "Fields in ms  = ", msFields

  # Check for mismatch
  if (bpolyOverlay):
      if (len(timerangeListTimes) > nUniqueTimesBP):
          print "There are more timeranges (%d) to plot from %s than exist in the caltable2=%s (%d)" % (len(timerangeListTimes), caltable,caltable2, nUniqueTimesBP)
          for i in timerangeList:
              if (sloppyMatch(timerangeListTimes[i],uniqueTimesBP[0],
                              solutionTimeThresholdSeconds)):
                  print "Try adding 'timeranges=%d'" % (i+1)
          return(vm)
      if (bpolyOverlay2):
          if (len(timerangeListTimes) > nUniqueTimesBP2):
              print "There are more timeranges to plot (%d) from %s than exist in the caltable3=%s (%d)" % (len(timerangeListTimes), caltable, caltable3, nUniqueTimesBP2)
              return(vm)
          
  # Parse the antenna string to emulate plotms
  if (type(antenna) == str):
     if (len(antenna) == sum([m in myValidCharacterListWithBang for m in antenna])):
         # a simple list of antenna numbers was given 
         tokens = antenna.split(',')
         antlist = []
         removeAntenna = []
         for token in tokens:
             if (len(token) > 0):
                 if (token.find('*')==0 and len(token)==1):
                     antlist = uniqueAntennaIds
                     break
                 elif (token.find('!')==0):
                     antlist = uniqueAntennaIds
                     removeAntenna.append(int(token[1:]))
                 elif (token.find('~')>0):
                     (start,finish) = token.split('~')
                     antlist +=  range(int(start),int(finish)+1)
                 else:
                     antlist.append(int(token))
         antlist = np.array(antlist)
         for rm in removeAntenna:
             antlist = antlist[np.where(antlist != rm)[0]]
         antlist = list(antlist)
         if (len(antlist) < 1 and len(removeAntenna)>0):
             print "Too many negated antennas -- there are no antennas left to plot."
             return(vm)
     else:
         # The antenna name (or list of names) was specified
         tokens = antenna.split(',')
         if (msFound):
             antlist = []
             removeAntenna = []
             for token in tokens:
                 if (token in vm.uniqueAntennas):
                     antlist = list(antlist)  # needed in case preceding antenna had ! modifier
                     antlist.append(vm.getAntennaIdsForAntennaName(token))
                 elif (token[0] == '!'):
                     if (token[1:] in vm.uniqueAntennas):
                         antlist = uniqueAntennaIds
                         removeAntenna.append(vm.getAntennaIdsForAntennaName(token[1:]))
                     else:
                         print "Antenna %s is not in the ms. It contains: " % (token), vm.uniqueAntennas
                         return(vm)
                 else:
                     print "Antenna %s is not in the ms. It contains: " % (token), vm.uniqueAntennas
                     return(vm)
             antlist = np.array(antlist)
             for rm in removeAntenna:
                 antlist = antlist[np.where(antlist != rm)[0]]
             antlist = list(antlist)
             if (len(antlist) < 1 and len(removeAntenna)>0):
                 print "Too many negated antennas -- there are no antennas left to plot."
                 return(vm)
         else:
             print "Antennas cannot be specified my name if the ms is not found."
             return(vm)
  elif (type(antenna) == list):
      # it's a list of integers
      antlist = antenna
  else:
      # It's a single, integer entry
      antlist = [antenna]

  if (len(antlist) > 0):
     antennasToPlot = np.intersect1d(uniqueAntennaIds,antlist)
  else:
     antennasToPlot = uniqueAntennaIds

  # Parse the field string to emulate plotms
  removeField = []
  if (type(field) == str):
     if (len(field) == sum([m in myValidCharacterListWithBang for m in field])):
         # a list of field numbers was given
         tokens = field.split(',')
         fieldlist = []
         for token in tokens:
             if (token.find('*')>=0):
                 fieldlist = uniqueFields
                 break
             elif (token.find('!')==0):
                 fieldlist = uniqueFields
                 removeField.append(int(token[1:]))
             elif (len(token) > 0):
                 if (token.find('~')>0):
                     (start,finish) = token.split('~')
                     fieldlist +=  range(int(start),int(finish)+1)
                 else:
                     fieldlist.append(int(token))
         fieldlist = np.array(fieldlist)
         for rm in removeField:
             fieldlist = fieldlist[np.where(fieldlist != rm)[0]]
         fieldlist = list(fieldlist)
         if (len(fieldlist) < 1 and len(removeField)>0):
             print "Too many negated fields -- there are no fields left to plot."
             return(vm)
     else:
         # The field name (or list of names, or wildcard) was specified
         tokens = field.split(',')
         if (msFound):
             fieldlist = []
             removeField = []
             for token in tokens:
                 myloc = token.find('*')
                 if (myloc > 0):
                     for u in uniqueFields:
                         if (token[0:myloc]==vm.getFieldNamesForFieldId(u)[0:myloc]):
                             if (DEBUG):
                                 print "Found wildcard match = %s" % vm.getFieldNamesForFieldId(u)
                             fieldlist.append(u)
                         else:
                             if (DEBUG):
                                 print "No wildcard match with = %s" % vm.getFieldNamesForFieldId(u)
                 elif (myloc==0):
                     for u in uniqueFields:
                         fieldlist.append(u)
                 elif (token in vm.uniqueFields):
                     fieldlist = list(fieldlist)  # needed in case preceding field had ! modifier
                     fieldlist.append(vm.getFieldIdsForFieldName(token))
                 elif (token[0] == '!'):
                     if (token[1:] in vm.uniqueFields):
                         fieldlist = uniqueFields
                         removeField.append(vm.getFieldIdsForFieldName(token[1:]))
                     else:
                         print "Field %s is not in the ms. It contains: " % (token), vm.uniqueFields
                         return(vm)
                 else:
                     print "Field %s is not in the ms. It contains: " % (token), vm.uniqueFields
                     return(vm)
             fieldlist = np.array(fieldlist)
             for rm in removeField:
                 fieldlist = fieldlist[np.where(fieldlist != rm)[0]]
             fieldlist = list(fieldlist)
             if (len(fieldlist) < 1 and len(removeField)>0):
                 print "Too many negated fields -- there are no fields left to plot."
                 return(vm)
         else:
             print "Fields cannot be specified my name if the ms is not found."
             return(vm)
  elif (type(field) == list):
      # it's a list of integers
      fieldlist = field
  else:
      # It's a single, integer entry
      fieldlist = [field]

  if (len(fieldlist) > 0):
      if (DEBUG):
          print "Finding intersection of ", uniqueFields, fieldlist
      fieldsToPlot = np.intersect1d(uniqueFields,np.array(fieldlist))
      if (bOverlay):
          fieldsToPlot = np.intersect1d(np.union1d(uniqueFields,uniqueFields2),np.array(fieldlist))
      if (len(fieldsToPlot) < 1):
          print "Requested field not found in solution"
          return(vm)
  else:
      fieldsToPlot = uniqueFields  # use all fields if none are specified
      if (bOverlay):
          fieldsToPlot = np.union1d(uniqueFields,uniqueFields2)
      if (DEBUG):
          print "bOverlay = ", bOverlay
          print "set fieldsToPlot to uniqueFields = ", fieldsToPlot
  fieldIndicesToPlot = []

  if (showatmfield == ''):
      showatmfield = fieldsToPlot[0]
  else:
      if (str.isdigit(str(showatmfield))):
          showatmfield = int(str(showatmfield))
          if (showatmfield not in fieldsToPlot):
              print "The showatmfield (%d) is not in the list of fields to plot: " %(showatmfield), fieldsToPlot
              return(vm)
      else:
          showatmfieldName = showatmfield
          showatmfield = vm.getFieldIdsForFieldName(showatmfield)
          if (list(showatmfield) == []):
              print "The showatmfield (%s) is not in the ms." %(showatmfieldName)
              return(vm)
          if (type(showatmfield) == np.ndarray):
              # more than one field IDs exist for this source name, so pick the first
              showatmfield = showatmfield[0]
          if (showatmfield not in fieldsToPlot):
              print "The showatmfield (%d=%s) is not in the list of fields to plot: " %(showatmfield, showatmfieldName), fieldsToPlot
              return(vm)

  for i in fieldsToPlot:
      match = np.where(i==uniqueFields)[0]
      if (len(match) < 1 and bOverlay):
          match = np.where(i==uniqueFields2)[0]
      fieldIndicesToPlot.append(match[0])
      
  if (overlayTimes and len(fieldsToPlot)>1 and 1==0):
        print "Times cannot be overlaid if more than one field is specified"
        return(vm)
  if (overlayTimes and len(fieldsToPlot)>1):
      multiFieldsWithOverlayTime = True
  else:
      multiFieldsWithOverlayTime = False

#  print "Antennas to plot = ", antennasToPlot
  print "spws to plot = ", spwsToPlot
  print "Field IDs to plot: ", fieldsToPlot
#  print "Field indices to plot: ", fieldIndicesToPlot

  redisplay = False
  myap = 0  # this variable is necessary to make the 'b' option work for subplot=11, yaxis=both
            # It keeps track of whether 'amp' or 'phase' was the first plot on the page.

  # I added these two lines because Remy suggested it. But 2nd one causes a new window everytime.
  pb.ion()
#  pb.figure()

  newylimits = [LARGE_POSITIVE, LARGE_NEGATIVE]
  
  if (bpoly):
    # The number of polarizations cannot be reliably inferred from the shape of the GAIN column
    # in the caltable.  Must use the shape of the DATA column in the ms.
    nPolarizations = vm.nPolarizations
    nPolarizations2 = vm.nPolarizations
    print "nPolarizations from vm = ", nPolarizations
    (corr_type, corr_type_string, nPolarizations) = getCorrType(msName)
    if (corr_type_string == []):
        return(vm)
    polsToPlot = checkPolsToPlot(polsToPlot, corr_type_string)
    if (polsToPlot == []):
        return(vm)
    pb.clf()
    # Here we are only plotting one BPOLY solution, no overlays implemented (yet).
    overlayAntennas = False
    # rows in the table are: antennas 0..nAnt for first spw, antennas 0..nAnt for 2nd spw...
    pagectr = 0
    pages = []
    xctr = 0
    newpage = 1
    while (xctr < len(antennasToPlot)):
      xant = antennasToPlot[xctr]
      antstring = buildAntString(xant,msFound,msAnt)
      spwctr = 0
      while (spwctr < len(spwsToPlot)):
       ispw = spwsToPlot[spwctr]
       mytime = 0
       while (mytime < nUniqueTimes):
         if (len(uniqueTimes) > 0 and (sloppyMatch(mytime,timerangeList,solutionTimeThresholdSeconds,myprint=debugSloppyMatch)==False)):
            mytime += 1
            continue
         if (newpage == 1):
            pages.append([xctr,spwctr,mytime,0])
#            print "appending [%d,%d,%d,%d]" % (xctr,spwctr,mytime,0)
            newpage = 0
         antennaString = 'Ant%d: %s,  ' % (xant,antstring)
         for index in range(nRows):
            # Find this antenna, spw, and timerange combination in the table
            if (xant==ant[index] and sloppyMatch(uniqueTimes[mytime],times[index],solutionTimeThresholdSeconds,myprint=debugSloppyMatch) and
                (ispw == cal_desc_id[index]) and (fields[index] in fieldsToPlot)):
                fieldIndex = np.where(fields[index] == uniqueFields)[0]
                validDomain = [frequencyLimits[0,index], frequencyLimits[1,index]]
                if (msFound):
                      fieldString = msFields[uniqueFields[fieldIndex]][0]
                else:
                      fieldString = str(field)
#                timeString = ',  t%d/%d  %s UT' % (mytime+1,nUniqueTimes,utstring(uniqueTimes[mytime],xframeStart))
                timeString = ',  t%d/%d  %s' % (mytime+1,nUniqueTimes,utstring(uniqueTimes[mytime],3))
                if ((yaxis.find('amp')>=0 or amplitudeWithPhase) and myap==0):
                  xframe += 1
                  myUniqueColor = []
                  if (debug):
                      print "v) incrementing xframe to %d" % xframe
                  adesc = pb.subplot(xframe)
                  if (ispw==originalSpw[ispw]):
                      # all this was added mistakenly here.  If it causes a bug, remove it.
                      if (overlayTimes and len(fieldsToPlot) > 1):
                        indices = fstring = ''
                        for f in fieldIndicesToPlot:
                            if (f != fieldIndicesToPlot[0]):
                                indices += ','
                                fstring += ','
                            indices += str(uniqueFields[f])
                            if (msFound):
                                fstring += msFields[uniqueFields[f]]
                        if (len(fstring) > fstringLimit):
                            fstring = fstring[0:fstringLimit] + '...'
                        pb.title("%sspw%d,  fields %s: %s%s" % (antennaString,ispw,
                                indices, fstring, timeString), size=titlesize)
                      else:
                        pb.title("%sspw%d,  field %d: %s%s" % (antennaString,ispw,
                                uniqueFields[fieldIndex],fieldString,timeString), size=titlesize)
                  else:
                      if (overlayTimes and len(fieldsToPlot) > 1):
                        indices = fstring = ''
                        for f in fieldIndicesToPlot:
                            if (f != fieldIndicesToPlot[0]):
                                indices += ','
                                fstring += ','
                            indices += str(uniqueFields[f])
                            if (msFound):
                                fstring += msFields[uniqueFields[f]]
                        if (len(fstring) > fstringLimit):
                            fstring = fstring[0:fstringLimit] + '...'
                        pb.title("%sspw%d (%d),  fields %s: %s%s" % (antennaString,ispw,originalSpw[ispw],
                                indices, fstring, timeString), size=titlesize)
                      else:
                        pb.title("%sspw%d (%d),  field %d: %s%s" % (antennaString,ispw,originalSpw[ispw],
                                uniqueFields[fieldIndex],fieldString,timeString), size=titlesize)
                  amplitudeSolutionX = np.real(scaleFactor[index])+calcChebyshev(polynomialAmplitude[index][0:nPolyAmp[index]], validDomain, frequenciesGHz[index]*1e+9)
                  amplitudeSolutionY = np.real(scaleFactor[index])+calcChebyshev(polynomialAmplitude[index][nPolyAmp[index]:2*nPolyAmp[index]], validDomain, frequenciesGHz[index]*1e+9)
                  amplitudeSolutionX += 1 - np.mean(amplitudeSolutionX)
                  amplitudeSolutionY += 1 - np.mean(amplitudeSolutionY)
                  if (yaxis.lower().find('db') >= 0):
                      amplitudeSolutionX = 10*np.log10(amplitudeSolutionX)
                      amplitudeSolutionY = 10*np.log10(amplitudeSolutionY)
                  if (nPolarizations == 1):
                      pb.plot(frequenciesGHz[index], amplitudeSolutionX, '%s%s'%(xcolor,bpolymarkstyle))
                  else:
                      pb.plot(frequenciesGHz[index], amplitudeSolutionX, '%s%s'%(xcolor,bpolymarkstyle), frequenciesGHz[index], amplitudeSolutionY, '%s%s'%(ycolor,bpolymarkstyle))
                  if (plotrange[0] != 0 or plotrange[1] != 0):
                      SetNewXLimits([plotrange[0],plotrange[1]])
                  if (plotrange[2] != 0 or plotrange[3] != 0):
                      SetNewYLimits([plotrange[2],plotrange[3]])
                  xlim=pb.xlim()
                  ylim=pb.ylim()
                  ResizeFonts(adesc,mysize)
                  adesc.xaxis.grid(True,which='major')
                  adesc.yaxis.grid(True,which='major')
                  if (yaxis.lower().find('db')>=0):
                      pb.ylabel('Amplitude (dB)', size=mysize)
                  else:
                      pb.ylabel('Amplitude', size=mysize)
                  pb.xlabel('Frequency (GHz)', size=mysize)
                  if (xframe == firstFrame):
                      DrawBottomLegendPageCoords(msName, uniqueTimes[mytime], mysize)
                      pb.text(xstartTitle, ystartTitle,
                              '%s (degamp=%d, degphase=%d)'%(caltable,nPolyAmp[index]-1,
                              nPolyPhase[index]-1),size=mysize,
                              transform=pb.gcf().transFigure) 
                  # draw polarization labels
                  x0 = xstartPolLabel
                  y0 = ystartPolLabel
                  for p in range(nPolarizations):
                      if (corrTypeToString(corr_type[p]) in polsToPlot):
                          pb.text(x0, y0-0.03*subplotRows*p, corrTypeToString(corr_type[p])+'',
                                  color=pcolor[p],size=mysize, transform=pb.gca().transAxes)
                  if (xframe == 111 and amplitudeWithPhase):
                     if (len(figfile) > 0):
                         # We need to make a new figure page
                         plotfiles.append(makeplot(figfile,msFound,msAnt,
                                          overlayAntennas,pages,pagectr,
                                          density,interactive,antennasToPlot,
                                          spwsToPlot,overlayTimes,0))
                     donetime = timeUtilities.time()
                     if (interactive):
                        pb.draw()
#                        myinput = raw_input("(%.1f sec) Press return for next page (b for backwards, q to quit): "%(donetime-mytimestamp))
                        myinput = raw_input("Press return for next page (b for backwards, q to quit): ")
                     else:
                        myinput = ''
                     mytimestamp = timeUtilities.time()
                     if (myinput.find('q') >= 0):
                         return(vm)
                     if (myinput.find('b') >= 0):
                         if (pagectr > 0):
                             pagectr -= 1
                         #redisplay the current page by setting ctrs back to the value they had at start of that page
                         xctr = pages[pagectr][PAGE_ANT]
                         spwctr = pages[pagectr][PAGE_SPW]
                         mytime = pages[pagectr][PAGE_TIME]
                         myap = pages[pagectr][PAGE_AP]
                         xant = antennasToPlot[xctr]
                         ispw = spwsToPlot[spwctr]
                         redisplay = True
                     else:
                         pagectr += 1
                         if (pagectr >= len(pages)):
                               pages.append([xctr,spwctr,mytime,1])
#                               print "appending [%d,%d,%d,%d]" % (xctr,spwctr,mytime,1)
                               newpage = 0
                     pb.clf()

                if (yaxis.find('phase')>=0 or amplitudeWithPhase):
                  xframe += 1
                  myUniqueColor = []
#                  print "w) incrementing xframe to %d" % xframe
                  adesc = pb.subplot(xframe)
                  if (ispw==originalSpw[ispw]):
                        pb.title("%sspw%d,  field %d: %s%s" % (antennaString,ispw,
                               uniqueFields[fieldIndex],fieldString,timeString), size=titlesize)
                  else:
                        pb.title("%sspw%d (%d),  field %d: %s%s" % (antennaString,ispw,originalSpw[ispw],
                               uniqueFields[fieldIndex],fieldString,timeString), size=titlesize)
                  phaseSolutionX = calcChebyshev(polynomialPhase[index][0:nPolyPhase[index]], validDomain, frequenciesGHz[index]*1e+9) * 180/math.pi
                  phaseSolutionY = calcChebyshev(polynomialPhase[index][nPolyPhase[index]:2*nPolyPhase[index]], validDomain, frequenciesGHz[index]*1e+9) * 180/math.pi
                  if (nPolarizations == 1):
                      pb.plot(frequenciesGHz[index], phaseSolutionX, '%s%s'%(xcolor,bpolymarkstyle))
                  else:
                      pb.plot(frequenciesGHz[index], phaseSolutionX, '%s%s'%(xcolor,bpolymarkstyle), frequenciesGHz[index], phaseSolutionY, '%s%s'%(ycolor,bpolymarkstyle))
                  ResizeFonts(adesc,mysize)
                  adesc.xaxis.grid(True,which='major')
                  adesc.yaxis.grid(True,which='major')
                  pb.ylabel('Phase (deg)', size=mysize)
                  pb.xlabel('Frequency (GHz)', size=mysize)
                  if (plotrange[0] != 0 or plotrange[1] != 0):
                      SetNewXLimits([plotrange[0],plotrange[1]])
                  if (plotrange[2] != 0 or plotrange[3] != 0):
                      SetNewYLimits([plotrange[2],plotrange[3]])
                  if (amplitudeWithPhase and phase != ''):
                      if (phase[0] != 0 or phase[1] != 0):
                          SetNewYLimits(phase)
                  if (xframe == firstFrame):
                      pb.text(xstartTitle, ystartTitle,
                              '%s (degamp=%d, degphase=%d)'%(caltable,
                              nPolyAmp[index]-1,nPolyPhase[index]-1),
                              size=mysize, transform=pb.gcf().transFigure)
                  # draw polarization labels
                  x0 = xstartPolLabel
                  y0 = ystartPolLabel
                  for p in range(nPolarizations):
                      if (corrTypeToString(corr_type[p]) in polsToPlot):
                          pb.text(x0, y0-0.03*p*subplotRows, corrTypeToString(corr_type[p])+'',
                                  color=pcolor[p],size=mysize, transform=pb.gca().transAxes)
                      
         # end of 'for' loop over rows
         redisplay = False
         pb.subplots_adjust(hspace=myhspace, wspace=mywspace)
         if (xframe == lastFrame):
            if (len(figfile) > 0):
                plotfiles.append(makeplot(figfile,msFound,msAnt,
                                  overlayAntennas,pages,pagectr,
                                  density,interactive,antennasToPlot,
                                  spwsToPlot,overlayTimes,1))
            donetime = timeUtilities.time()
            if (interactive):
               pb.draw()
#               myinput = raw_input("(%.1f sec) Press return for next page (b for backwards, q to quit): "%(donetime-mytimestamp))
               myinput = raw_input("Press return for next page (b for backwards, q to quit): ")
            else:
               myinput = ''
            mytimestamp = timeUtilities.time()
            if (myinput.find('q') >= 0):
                return(vm)
            if (myinput.find('b') >= 0):
                if (pagectr > 0):
                    pagectr -= 1
                #redisplay the current page by setting ctrs back to the value they had at start of that page
                xctr = pages[pagectr][PAGE_ANT]
                spwctr = pages[pagectr][PAGE_SPW]
                mytime = pages[pagectr][PAGE_TIME]
                myap = pages[pagectr][PAGE_AP]
                xant = antennasToPlot[xctr]
                ispw = spwsToPlot[spwctr]
                redisplay = True
            else:
                pagectr += 1
                if (pagectr >= len(pages)):
                    newpage = 1
                else:
                    newpage = 0
            xframe = xframeStart
            if (xctr+1 < len(antennasToPlot)):
                # don't clear the final plot when finished
                pb.clf()
            if (spwctr+1<len(spwsToPlot) or mytime+1<nUniqueTimes):
                # don't clear the final plot when finished
                pb.clf()
            pb.subplots_adjust(hspace=myhspace, wspace=mywspace)
         if (redisplay == False):
             mytime += 1
             if (debug):
                 print "Incrementing mytime to %d" % (mytime)
       # end while(mytime)
       if (redisplay == False):
           spwctr +=1
           if (debug):
               print "Incrementing spwctr to %d" % (spwctr)
      # end while(spwctr)
      if (redisplay == False):
          xctr += 1
    # end while(xctr)
    if (len(figfile) > 0 and pagectr<len(pages)):
       plotfiles.append(makeplot(figfile,msFound,msAnt,overlayAntennas,pages,
                                 pagectr,density,interactive,antennasToPlot,
                                 spwsToPlot,overlayTimes,2))
    if (len(plotfiles) > 0 and buildpdf):
        pdfname = figfile+'.pdf'
        filelist = ''
        plotfiles = np.unique(plotfiles)
        for i in range(len(plotfiles)):
            mystatus = os.system('%s -density %d %s %s.pdf' % (convert,density,plotfiles[i],plotfiles[i].split('.png')[0]))
            if (mystatus != 0):
                break
#            filelist += plotfiles[i].split('.png')[0] + '.pdf '
            filelist += plotfiles[i] + '.pdf '
        if (mystatus != 0):
            print "ImageMagick is missing, no PDF created"
            buildpdf = False
        if (buildpdf==True):
            cmd = '%s -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s' % (gs,pdfname,filelist)
            print "Running command = %s" % (cmd)
            mystatus = os.system(cmd)
            if (mystatus == 0):
                print "PDF left in %s" % (pdfname)
                os.system("rm -f %s" % filelist)
            else:
                print "ghostscript is missing, no PDF created"
    return(vm)

##################################################################################
# bpoly == false
###############
  msFound = False
  tb.open(caltable)
  if (ParType == 'Complex'):
      gain = tb.getcol('CPARAM')
  else:
      gain = tb.getcol('FPARAM')
  nPolarizations =  len(gain)
  if (debug):
      print "(1)Set nPolarizations = %d" % nPolarizations
  ggx = gain[0][:][:]
  if (nPolarizations == 2):
        ggy = gain[1][:][:]
  tb.close()

# This no longer works in 3.4, it gives npol=1 for TWHya data.
#  try:
#      nPolarizations = vm.nPolarizations
#  except:
#      print "You need to locate the parent ms, and pass it in via ms=''."
#      return
  if (debug):
    print "nPolarizations = ", nPolarizations
  nRows = len(gain[0][0])
  if (bOverlay):
        tb.open(caltable2)
        if (ParType == 'Complex'):
            gain2 = tb.getcol('CPARAM')
        else:
            gain2 = tb.getcol('FPARAM')
        ggx2 = gain2[0][:][:]
        tb.close()
        nPolarizations2 = len(gain2)
        if (nPolarizations == 2):
              ggy2 = gain2[1][:][:]
        nRows2 = len(gain2[0][0])

  try:
      if (DEBUG):
          print "Trying to open %s" % (msName+'/SPECTRAL_WINDOW')
      tb.open(msName+'/SPECTRAL_WINDOW')
      refFreq = tb.getcol('REF_FREQUENCY')
      net_sideband = tb.getcol('NET_SIDEBAND')
      measFreqRef = tb.getcol('MEAS_FREQ_REF')
      tb.close()
      
      (corr_type, corr_type_string, nPolarizations) = getCorrType(msName)
      if (corr_type_string == []):
          return(vm)
  except:
      print "2) Could not open the associated measurement set tables (%s). Will not translate antenna names." % (msName)
      print "I will assume ALMA data: XX, YY, and refFreq=first channel."
      corr_type_string = ['XX','YY']
      corr_type = [9,12]

  if (len(polsToPlot) > len(corr_type)):
      # Necessary for SMA (single-pol) data
      polsToPlot = corr_type_string
  print "Polarizations to plot = ", polsToPlot
  polsToPlot = checkPolsToPlot(polsToPlot, corr_type_string)
  if (polsToPlot == []):
      return(vm)

  if (len(msAnt) > 0):
      msFound = True
  else:
      if (xaxis.find('freq')>=0):
          print "Because I could not open the .ms, you cannot use xaxis='freq'."
          return(vm)
  
  if (bpoly == False):
      if (debug):
          print "nPolarizations = ", nPolarizations
          print "nFields = %d = " % (nFields), uniqueFields

  if (bOverlay and debug):
        print "nPolarizations2 = ", nPolarizations2
        print "nFields2 = %d = " % (nFields2), uniqueFields2
        print "nRows2 = ", nRows2
  uniqueAntennaIds = np.sort(np.unique(ant))

  if (VisCal.lower().find('tsys') >= 0):
      yAmplitudeLabel = "Tsys (K)"
  else:
      if (yaxis.lower().find('db')>=0):
          yAmplitudeLabel = "Amplitude (dB)"
      else:
          yAmplitudeLabel = "Amplitude"

  ampMin = LARGE_POSITIVE
  ampMax = LARGE_NEGATIVE

  pb.clf()
  TDMisSecond = False
  pagectr = 0
  newpage = 1
  pages =  []
  xctr = 0
  myap = 0  # determines whether an amp or phase plot starts the page (in the case of 'both')
  redisplay = False
  matchctr = 0
  myUniqueColor = []
  # for the overlay=antenna case, start by assuming the first antenna is not flagged
  firstUnflaggedAntennaToPlot = 0
  lastUnflaggedAntennaToPlot = len(antennasToPlot)
  computedAtmSpw = -1
  computedAtmTime = -1
  computedAtmField = -1
  while (xctr < len(antennasToPlot)):
    xant = antennasToPlot[xctr]
    spwctr = 0
    antstring = buildAntString(xant,msFound,msAnt)
    while (spwctr < len(spwsToPlot)):
      ispw = spwsToPlot[spwctr]
      ispwInCalTable = list(uniqueSpwsInCalTable).index(ispw)
#      print "ispw=%d, len(chanFreqGHz) = %d" % (ispw, len(chanFreqGHz))
      nChannels = len(chanFreqGHz[ispw])
      nChannels2 = len(chanFreqGHz2[ispw])
      mytime = 0
      if (overlayAntennas):
          xctr = -1
      firstTimeMatch = -1
#      computedAtm = -1  # only compute once per frame (obsolete)
      while (mytime < nUniqueTimes):
        if (debug):
            print "mytime = %d < %d" % (mytime,nUniqueTimes)
#        if (len(timerangeList) > 0 and (sloppyMatch(mytime,timerangeList,solutionTimeThresholdSeconds,myprint=debugSloppyMatch)==False)):
        if (len(timerangeList) > 0 and (sloppyMatch(uniqueTimes[mytime],timerangeListTimes,solutionTimeThresholdSeconds,myprint=debugSloppyMatch)==False)):
            if (debug):
                print "Skipping time %d because it is not in the list: " % (mytime), timerangeList
            mytime += 1
            continue
        if (overlayAntennas):
            xctr += 1
            if (xctr >= len(antennasToPlot)):
                xctr = 0
            xant = antennasToPlot[xctr]
            if (debug):
                print "mytime=%d, Set xant to %d" % (mytime,xant)
            antennaString = ''
        else:
            antennaString = 'Ant%d: %s,  ' % (xant,antstring)
        # This used to be above the previous if/else block
        if (newpage==1):
            # add the current page (being created here) to the list
            pages.append([xctr,spwctr,mytime,0])
            if (debug):
                print "top: appending [%d,%d,%d,%d]" % (xctr,spwctr,mytime,0)
            newpage = 0
        gplotx = []
        gploty = []
        channels = []
        xchannels = []
        ychannels = []
        frequencies = []
        xfrequencies = []
        yfrequencies = []
        channels2 = []
        xchannels2 = []
        ychannels2 = []
        frequencies2 = []
        xfrequencies2 = []
        yfrequencies2 = []
        gplotx2 = []
        gploty2 = []
        xflag = []
        yflag = []
        xflag2 = []
        yflag2 = []
        matchFound = False
        for i in range(nRows):
            sm = sloppyMatch(uniqueTimes[mytime],times[i],solutionTimeThresholdSeconds,myprint=False)
            if ((ant[i]==xant) and (cal_desc_id[i]==ispw) and sm
                and (mytime in timerangeList)   # this test was added to support multiFieldInTimeOverlay
                ):
                if (fields[i] in fieldsToPlot):
                    interval = intervals[i] # used for CalcAtmTransmission
                    fieldIndex = np.where(fields[i] == uniqueFields)[0]
                    if (debug):
                        print "%d Found match at field,ant,spw,mytime,time = %d(index=%d),%d,%d,%d,%f=%s" % (matchctr,fields[i],fieldIndex,xant,ispw,mytime,uniqueTimes[mytime],utstring(uniqueTimes[mytime],4))
                    matchFound = True
                    xflag.append(flags[0,:,i])
                    yflag.append(flags[1,:,i])
                    for j in range(len(chanFreqGHz[ispw])):
                        channels.append(j)
                        if (msFound):
#                            print "ispw=%d, j=%d, len(chanFreqGHz) = " % (ispw,j), len(chanFreqGHz)
                            frequencies.append(chanFreqGHz[ispw][j])
                        if (showflagged or (showflagged == False and flags[0,j,i]==0)):
                            gplotx.append(ggx[j][i])
                            xchannels.append(j)
                            if (msFound):
                                xfrequencies.append(chanFreqGHz[ispw][j])
                        if (nPolarizations == 2):
                            if (showflagged or (showflagged == False and flags[1,j,i]==0)):
                                gploty.append(ggy[j][i])
                                ychannels.append(j)
                                if (msFound):
                                  yfrequencies.append(chanFreqGHz[ispw][j])
        # end 'for i'
        myspw = originalSpw[ispw]
        if (myspw >= len(refFreq)):
            myspw = ispw
        if (msFound and refFreq[myspw]*1e-9 > 60):
          # Then this cannot be EVLA data.  But I should really check the telescope name!
          if (refFreq[myspw]*1e-9 > np.mean(frequencies)):
              sideband = -1
              xlabelString = "%s LSB Frequency (GHz)" % refTypeToString(measFreqRef[myspw])
          else:
              sideband = +1
              xlabelString = "%s USB Frequency (GHz)" % refTypeToString(measFreqRef[myspw])
        else:
            sideband = -1
            xlabelString = "Frequency (GHz)"
        if ((len(frequencies)>0) and (chanrange[1] > len(frequencies))):
            print "Invalid chanrange for spw%d in caltable1. Valid range = 0-%d" % (ispw,len(frequencies))
            return(vm)
        pchannels = [xchannels,ychannels]
        pfrequencies = [xfrequencies,yfrequencies]
        gplot = [gplotx,gploty]
        if ((showatm or showtsky) and (len(xchannels)>0 or len(ychannels)>0) and
            (uniqueFields[fieldIndex]==showatmfield or overlayTimes==False) and
            ((overlayTimes==False and computedAtmField!=fieldIndex) or (computedAtmSpw!=ispw) or
             (overlayTimes==False and computedAtmTime!=mytime))):
            #       uniqueFields[fieldIndex]):
#            print "CAF, CAS, CAT = ", computedAtmField, computedAtmSpw, computedAtmTime
            computedAtmField = fieldIndex
            computedAtmSpw = ispw
            computedAtmTime = mytime
            atmtime = timeUtilities.time()
            asdm = ''
            (atmfreq,atmchan,transmission,pwvmean,atmairmass,TebbSky,missingCalWVRErrorPrinted) = \
               CalcAtmTransmission(pchannels, pfrequencies, xaxis, pwv,
                       vm, msName, asdm, xant, uniqueTimes[mytime],
                       interval, uniqueFields[fieldIndex],
                       refFreq[originalSpw[ispw]],
                       net_sideband[originalSpw[ispw]], mytime, 
                       missingCalWVRErrorPrinted, verbose=DEBUG)
            if (showimage):
                returnValue = au.getLOs(msName, verbose=False)
                if (returnValue == []):
                    if (lo1 == None):
                        print "Since you do not have the ASDM_RECEIVER table, you must specify the LO1 frequency with lo1=."
                        return(vm)
                    LO1 = lo1
                else:
                    [LOs,bands,spws,names,sidebands,receiverIds] = returnValue
                    spws = fixGapInSpws(spws, DEBUG)
                    LO1 = LOs[spws.index(originalSpw[ispw])][0] * 1e-9
                    print "Found LO1 = %.6f GHz" % (LO1)
            if (LO1 != None):
#                print "Computing image frequencies (LO1=%f) ***********************************"%(LO1)
                xfrequenciesImage = list(2*LO1 - np.array(pfrequencies[0]))
                yfrequenciesImage = list(2*LO1 - np.array(pfrequencies[1]))
                pfrequenciesImage = [xfrequenciesImage, yfrequenciesImage]
                (atmfreqImage,atmchanImage,transmissionImage,pwvmean,atmairmass,TebbSkyImage,missingCalWVRErrorPrinted) = \
                    CalcAtmTransmission(pchannels, pfrequenciesImage, xaxis,
                              pwv, vm, msName, asdm, xant, uniqueTimes[mytime],
                              interval, uniqueFields[fieldIndex],
                              refFreq[originalSpw[ispw]],
                              net_sideband[originalSpw[ispw]], mytime, 
                              missingCalWVRErrorPrinted, verbose=DEBUG)
                atmfreqImage = list(2*LO1 - np.array(atmfreqImage))
                atmfreqImage.reverse()
                atmchanImage.reverse()

#            print "len(TebbSky)=%d, len(transmission)=%d" % (len(TebbSky),len(transmission))
            if (overlayTimes):
                atmString = 'PWV %.2fmm, airmass %.2f (field %d)' % (pwvmean,atmairmass,showatmfield)
            else:
                atmString = 'PWV %.2fmm, airmass %.3f' % (pwvmean,atmairmass)
#                atmString = 'PWV %.2fmm, airmass %.3f (field %d)' % (pwvmean,atmairmass,uniqueFields[fieldIndex])

#            print "ATM calc = %.2f sec" % (timeUtilities.time()-atmtime)
#            if (overlayTimes):
#                computedAtm = xframe+1 #   obsolete
#            else:
#                computedAtm = xframe #     obsolete
        if (bOverlay):
          for i in range(nRows2):
            if ((ant2[i]==xant) and (cal_desc_id2[i]==ispw) and sloppyMatch(uniqueTimes2[mytime],times2[i],solutionTimeThresholdSeconds,myprint=debugSloppyMatch)):
                if (fields2[i] in fieldsToPlot):
                      xflag2.append(flags2[0,:,i])
                      yflag2.append(flags2[1,:,i])
                      # With solint='2ch' or more, the following loop should not be over
                      # chanFreqGHz2 but over the channels in the solution.
                      for j in range(len(chanFreqGHz2[ispw])):
                        channels2.append(j)
                        frequencies2.append(chanFreqGHz2[ispw][j])
                        if (showflagged or (showflagged == False and flags2[0,j,i]==0)):
                            gplotx2.append(ggx2[j][i])
                            xchannels2.append(j)
                            xfrequencies2.append(chanFreqGHz2[ispw][j])
                        else:
                            if (debug):
                                print "********* flags2[0,%d,%d] = %d, showflagged=" % (j,i,flags2[0,j,i]), showflagged
                        if (nPolarizations2 == 2):
                            if (showflagged or (showflagged == False and flags2[1,j,i]==0)):
                                gploty2.append(ggy2[j][i])
                                ychannels2.append(j)
                                yfrequencies2.append(chanFreqGHz2[ispw][j])
          # end 'for i'
          pchannels2 = [xchannels2,ychannels2]
          pfrequencies2 = [xfrequencies2,yfrequencies2]
          gplot2 = [gplotx2,gploty2]
          if (len(frequencies2)>0 and (chanrange[1] > len(frequencies2))):
              print "Invalid chanrange for spw%d in caltable2. Valid range = 0-%d" % (ispw,len(frequencies2))
              return(vm)
              
# Prevents crash if long value is set for solutionTimeThresholdSeconds, but prints a lot of
# messages for Tsys with overlay='antenna'.
#        if (len(xchannels) < 1):
#            print "No unflagged data found for (ant,spw,mytime,time) = %d,%d,%d,%.1f=%s" % (xant,ispw,mytime,uniqueTimes[mytime],utstring(uniqueTimes[mytime],4))
#            matchFound = False

        if (matchFound==False):
            if (overlayAntennas==False or (overlayAntennas==True and xctr+1 >= len(antennasToPlot))):
                mytime += 1
                if (debug):
                    print "a) xctr=%d, Incrementing mytime to %d" % (xctr, mytime)
            continue
        #  The following variable allows color legend of UT times to match line plot
        myUniqueTime = []
        if (multiFieldsWithOverlayTime):
            # support multi-fields with overlay='time'
            uTPFPS = []
            for f in fieldIndicesToPlot:
                for t in uniqueTimesPerFieldPerSpw[ispwInCalTable][f]:
                    if (sloppyMatch(t, timerangeListTimes, solutionTimeThresholdSeconds,myprint=debugSloppyMatch)):
                        uTPFPS.append(t)
            uTPFPS = np.sort(uTPFPS)
            ctr = 0
            for t in uTPFPS:
                if (sloppyMatch(t, uniqueTimes[mytime], solutionTimeThresholdSeconds,myprint=debugSloppyMatch)):
                    myUniqueTime = mytime
                    ctr += 1
            if (ctr > 1):
                print "multi-field time overlay ***************  why are there 2 matches?"
#            if (ctr == 0):
#                print "No match for %.1f in "%(t), uTPFPS
        else:
#          print "ispw=%d, fieldIndex=%d, len(uTPFPS)=%d" % (ispw,fieldIndex,len(uniqueTimesPerFieldPerSpw))
          for t in uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex]:
            if (sloppyMatch(t, uniqueTimes[mytime], solutionTimeThresholdSeconds,myprint=debugSloppyMatch) == True):
                myUniqueTime = mytime  # was t
        
#        print "Overlay antenna %d, myUniqueTime=%d" % (xctr, myUniqueTime)
        if (xframe == xframeStart):
              pb.clf()
        xflag = [item for sublist in xflag for item in sublist]
        yflag = [item for sublist in yflag for item in sublist]
        pflag = [xflag, yflag]
        flagfrequencies = [frequencies, frequencies2]
        antstring = buildAntString(xant,msFound,msAnt)
        if (msFound):
              fieldString = msFields[uniqueFields[fieldIndex]][0]
        else:
              fieldString = str(field)
        if (overlayTimes):
            timeString =''
        else:
#            timeString = ',  t%d/%d  %s UT' % (mytime+1,nUniqueTimes,utstring(uniqueTimes[mytime],xframeStart))
            timeString = ',  t%d/%d  %s' % (mytime+1,nUniqueTimes,utstring(uniqueTimes[mytime],3))
        if (ispw==originalSpw[ispw]):
              titleString = "%sspw%d,  field %d: %s%s" % (antennaString,ispw,uniqueFields[fieldIndex],fieldString,timeString)
        else:
              titleString = "%sspw%d (%d),  field %d: %s%s" % (antennaString,ispw,originalSpw[ispw],uniqueFields[fieldIndex],fieldString,timeString)
        if (sum(xflag)==nChannels and sum(yflag)==nChannels and showflagged==False):
                if (overlayTimes):
                   print "Skip %s (%s) for time%d=%s all data flagged" % (antstring, titleString,mytime,utstring(uniqueTimes[mytime],3))
                else:
                   print "Skip %s (%s) all data flagged" % (antstring, titleString)
                if (overlayAntennas==False):
                    mytime += 1
                    if (debug):
                      print "F) incrementing mytime to %d" % mytime
#                elif (xctr+1 >= len(antennasToPlot)): # Try commenting this out on Apr 2, 2012.  Seems good.
#                    mytime = mytime+1 
#                    if (debug):
#                        print "x) incrementing mytime to %d (xctr=%d)" % (mytime,xctr)
#                else:
#                    if (debug):
#                        print "x) not incrementing mytime because xctr=%d+1 < len(antennasToPlot)=%d" % (xctr, len(antennasToPlot))
                if (overlayAntennas):
                      if (xctr == firstUnflaggedAntennaToPlot):
                            firstUnflaggedAntennaToPlot += 1
                            if (firstUnflaggedAntennaToPlot >= len(antennasToPlot)):
                                firstUnflaggedAntennaToPlot = 0
                                mytime += 1
                            if (debug):
                                print "----- Resetting firstUnflaggedAntennaToPlot from %d to %d = %d" % (firstUnflaggedAntennaToPlot-1, firstUnflaggedAntennaToPlot, antennasToPlot[firstUnflaggedAntennaToPlot])
                            continue # Try this on Apr 2, 2012 to fix bug.
                if (overlayAntennas==False and subplot==11):
                      # added the case (subplot==11) on April 22, 2012 to prevent crash on multi-antenna subplot=421
                      if (debug):
                          print "#######  removing [%d,%d,%d,%d]" % (pages[len(pages)-1][PAGE_ANT],
                                                            pages[len(pages)-1][PAGE_SPW],
                                                            pages[len(pages)-1][PAGE_TIME],
                                                            pages[len(pages)-1][PAGE_AP])
                      pages = pages[0:len(pages)-1]
                      newpage = 1
                if (overlayAntennas==False):
                      continue
                
        if (firstTimeMatch == -1):
              firstTimeMatch = mytime
#              print "Setting firstTimeMatch from -1 to ", firstTimeMatch

######### Here is the amplitude plotting ############
        if (yaxis.find('amp')>=0 or yaxis.find('both')>=0 or yaxis.find('ap')>=0):

          if (debug):
              print "amp: xctr=%d, xant=%d, myap=%d, mytime=%d, firstTimeMatch=%d, bOverlay=" % (xctr, xant, myap, mytime, firstTimeMatch), bOverlay
          if (myap==1):
            if (overlayTimes == False or mytime==firstTimeMatch):
                if (overlayAntennas==False or xctr==firstUnflaggedAntennaToPlot
                    or xctr==antennasToPlot[-1]):  # 2012-05-24, to fix the case where all ants flagged on one timerange
                    xframe += 1
                    if (debug):
                        print "y) incrementing xframe to %d" % xframe
                        print "mytime == firstTimeMatch", firstTimeMatch
                    myUniqueColor = []
                    newylimits = [LARGE_POSITIVE, LARGE_NEGATIVE]
          else: # (myap == 0)
            if (overlayTimes == False or mytime==firstTimeMatch):
                if (overlayAntennas==False or xctr==firstUnflaggedAntennaToPlot
                    or xctr>antennasToPlot[-1]):  # 2012-05-24, to fix the case where all ants flagged on one timerange
                    xframe += 1
                    if (debug):
                        print "Y) incrementing xframe to %d" % xframe
                    myUniqueColor = []
                    newylimits = [LARGE_POSITIVE, LARGE_NEGATIVE]
#                    print "myap=%d, mytime == firstTimeMatch" % myap, firstTimeMatch
            if (debug):
                print "$$$$$$$$$$$$$$$$$$$$$$$  ready to plot amp on xframe %d" % (xframe)
#            print ",,,,,,,,,,,,,,,, Starting with newylimits = ", newylimits
            adesc = pb.subplot(xframe)
            pb.hold(overlayAntennas or overlayTimes)
            gampx = np.abs(gplotx)
            if (nPolarizations == 2):
                gampy = np.abs(gploty)
                if (yaxis.lower().find('db') >= 0):
                    gamp = [10*np.log10(gampx), 10*np.log10(gampy)]
                else:
                    gamp = [gampx,gampy]
            else:
                if (yaxis.lower().find('db') >= 0):
                    gamp = [10*np.log10(gampx)]
                else:
                    gamp = [gampx]
            if (bOverlay):
                  gampx2 = np.abs(gplotx2)
                  if (nPolarizations2 == 2):
                        gampy2 = np.abs(gploty2)
                        gamp2 = [gampx2,gampy2]
                  else:
                        gamp2 = [gampx2]
            if (xaxis.find('chan')>=0 or msFound==False):    #  'amp'
                if (debug):
                    print "amp: plot vs. channel **********************"
                pb.hold(True)
                for p in range(nPolarizations):
                    if (overlayAntennas or overlayTimes):
                        if (corr_type_string[p] in polsToPlot):
                              pdesc = pb.plot(pchannels[p],gamp[p],'%s'%ampmarkstyles[p],
                                              markersize=markersize,
                                              markerfacecolor=overlayColors[xctr])
                              newylimits =  recalcYlimits(plotrange,newylimits,gamp[p])
                              if (overlayAntennas):
                                    pb.setp(pdesc, color=overlayColors[xctr])
                              else: # overlayTimes
                                  if (myUniqueTime != [] and (len(fieldsToPlot) > 1 or len(timerangeList)>1)):
                                      pb.setp(pdesc, color=overlayColors[myUniqueTime])
#                                      print "pb.setp: myUniqueTime, overlayColors = ", myUniqueTime, overlayColors[myUniqueTime]
                                      if (p==0 or len(polsToPlot)==1):
                                          myUniqueColor.append(overlayColors[len(myUniqueColor)])
                                      pb.setp(pdesc, color=myUniqueColor[-1])
                    else:
                        if (corr_type_string[p] in polsToPlot):
#                          print "pcolor[%d]=%s" % (p,pcolor)
                          pb.plot(pchannels[p],gamp[p],'%s%s'%(pcolor[p],ampmarkstyle), markersize=markersize)
                          newylimits =  recalcYlimits(plotrange,newylimits,gamp[p])
                if (sum(xflag)>0):
                    xrange = np.max(channels)-np.min(channels)
                    SetNewXLimits([np.min(channels)-xrange/20, np.max(channels)+xrange/20])
#                    print "amp: Resetting xaxis channel range to counteract flagged data"
                if (xframe in bottomRowFrames or (xctr+1==len(antennasToPlot) and ispw==spwsToPlot[-1])):
                    pb.xlabel("Channel", size=mysize)
            elif (xaxis.find('freq')>=0):   # amp
                if (bOverlay):
                      pb.hold(True)
                      xrange = np.abs(xfrequencies[0]-xfrequencies[-1])
                      try:
                          xrange2 = np.abs(xfrequencies2[0]-xfrequencies2[-1])
                      except:
                          print "No amp data found in second solution.  Try increasing the solutionTimeThresholdSeconds above %.0f." % (solutionTimeThresholdSeconds)
                          print "If this doesn't work, email the developer (%s)." % (developerEmail)
                          return(vm)

                      if (np.abs(xrange/xrange2 - 1) > 0.05 + len(xflag)/len(xchannels)):  # 0.0666 is 2000/1875-1
                         # These line widths are optimal for visualizing FDM over TDM
                         width1 = 1
                         width2 = 4
                         # solutions differ in frequency width
                         if (xrange < xrange2):
                            for p in range(nPolarizations):
                                  if (corrTypeToString(corr_type[p]) in polsToPlot):
                                        pb.plot(pfrequencies[p], gamp[p], '%s%s'%(pcolor[p],ampmarkstyle), linewidth=width2)
                                        newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                            for p in range(nPolarizations):
                                  if (corrTypeToString(corr_type[p]) in polsToPlot):
                                        pb.plot(pfrequencies2[p], gamp2[p], '%s%s'%(p2color[p],ampmarkstyle), linewidth=width1)
                                        newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp2[p], sideband,plotrange,xchannels2)
                         else:
                            for p in range(nPolarizations):
                                  if (corrTypeToString(corr_type[p]) in polsToPlot):
                                        pb.plot(pfrequencies2[p], gamp2[p], '%s%s'%(p2color[p],ampmarkstyle), linewidth=width2)
                                        newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp2[p], sideband,plotrange,xchannels2)
                            for p in range(nPolarizations):
                                  if (corrTypeToString(corr_type[p]) in polsToPlot):
                                        pb.plot(pfrequencies[p], gamp[p], '%s%s'%(pcolor[p],ampmarkstyle), linewidth=width1)
                                        newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                      else:
                         width1 = 1
                         width2 = 1
                         # solutions may be different level of smoothing, so plot highest rms first
                         if (np.std(gamp[0]) < np.std(gamp2[0])):
                            for p in range(nPolarizations):
                                if (corrTypeToString(corr_type[p]) in polsToPlot):
                                    pb.plot(pfrequencies2[p], gamp2[p], '%s%s'%(p2color[p],ampmarkstyle), linewidth=width1)
                                    newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp2[p], sideband,plotrange,xchannels2)
                            for p in range(nPolarizations):
                                if (corrTypeToString(corr_type[p]) in polsToPlot):
                                    pb.plot(pfrequencies[p], gamp[p], '%s%s'%(pcolor[p],ampmarkstyle), linewidth=width2)
                                    newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                         else:
                            for p in range(nPolarizations):
                                if (corrTypeToString(corr_type[p]) in polsToPlot):
                                    pb.plot(pfrequencies[p], gamp[p], '%s%s'%(pcolor[p],ampmarkstyle), linewidth=width2)
                                    newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                            for p in range(nPolarizations):
                                if (corrTypeToString(corr_type[p]) in polsToPlot):
                                    pb.plot(pfrequencies2[p], gamp2[p], '%s%s'%(p2color[p],ampmarkstyle), linewidth=width1)
                                    newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp2[p], sideband,plotrange,xchannels2)
                      # must set new limits after plotting  'amp'
                      if (zoom=='intersect'):
                          if (xrange < xrange2):
                              SetNewXLimits([min(xfrequencies[0],xfrequencies[-1])-xrange*0.1, max(xfrequencies[0],xfrequencies[-1])+xrange*0.1])
                              SetLimits(plotrange, chanrange, newylimits, channels, frequencies,
                                        pfrequencies, ampMin, ampMax, xaxis, pxl)
                          else:
                              SetNewXLimits([min(xfrequencies2[0],xfrequencies2[-1])-xrange2*0.1, max(xfrequencies2[0],xfrequencies2[-1])+xrange2*0.1])
                              slstatus = SetLimits(plotrange, chanrange, newylimits, channels, frequencies2,
                                        pfrequencies2, ampMin, ampMax, xaxis, pxl)
                      else:
                          if (xrange < xrange2):
                              SetLimits(plotrange, chanrange, newylimits, channels, frequencies,
                                        pfrequencies, ampMin, ampMax, xaxis, pxl)
                          else:
                              SetLimits(plotrange, chanrange, newylimits, channels, frequencies2,
                                        pfrequencies2, ampMin, ampMax, xaxis, pxl)
                      # draw polarization and spw labels
                      if (xframe == firstFrame):
                          # draw title including caltable name
                          caltableList = 'c1 = ' + caltable + ',  c2 = ' + caltable2 + ' (%s)'%(utstring(uniqueTimes2[mytime],3))
                          pb.text(xstartTitle, ystartTitle, caltableList, size=titlesize,
                                  color='k', transform=pb.gcf().transFigure)
                elif (bpolyOverlay):
                    if (debug):
                        print "in bpolyOverlay **********************************"
                    matches1 = []
                    for tbp in range(len(timesBP)):
                        if (sloppyMatch(uniqueTimes[mytime], timesBP[tbp], solutionTimeThresholdSeconds,myprint=debugSloppyMatch)):
                            matches1.append(tbp)
                    matches1 = np.array(matches1)
                    if (len(matches1) < 1):
                        print "No time match found between %.1f and " % (uniqueTimes[mytime]), timesBP
                        print "If you are sure the solutions correspond to the same data, you can set solutionTimeThresholdSeconds>=%.0f" % (1+np.ceil(np.abs(timesBP[0]-uniqueTimes[mytime])))
                        return(vm)
                    matches2 = np.where(xant == np.array(antennasBP))[0]
                    if (len(matches2) < 1):
                        print "No antenna match found: ", xant, antennasBP
                    matches3 = np.where(ispw == np.array(cal_desc_idBP))[0]
                    if (len(matches3) < 1):
                        print "No spw match found: ", ispw, cal_desc_idBP
                    matches12 = np.intersect1d(matches1,matches2)
                    if (len(matches12) < 1):
                        print "No time+antenna match between: ", matches1, matches2
                    matches = np.intersect1d(matches12, matches3)
                    if (len(matches) < 1):
                        print "No time+antenna+spw match between: ", matches12, matches3
                    try:
                        index = matches[0]
                        if (debug):
                            print "Match = %d ***********************************" % (index)
                    except:
                        print "No match found for time=%.6f, xant=%d, ispw=%d"  % (uniqueTimes[mytime],xant,ispw)
                        print "antennasBP = ", antennasBP
                        print "cal_desc_idBP = ", cal_desc_idBP
                        print "timesBP = "
                        for i in timesBP:
                            print "%.6f, " % i
                        return(vm)
                    validDomain = [frequencyLimits[0,index], frequencyLimits[1,index]]
                    cc = calcChebyshev(polynomialAmplitude[index][0:nPolyAmp[index]], validDomain, frequenciesGHz[index]*1e+9)
                    fa = np.array(frequenciesGHz[index])
                    if (xfrequencies[0] < xfrequencies[-1]):
                        matches = np.where(fa>xfrequencies[0])[0]
                        matches2 = np.where(fa<xfrequencies[-1])[0]
                    else:
                        matches = np.where(fa>xfrequencies[-1])[0]
                        matches2 = np.where(fa<xfrequencies[0])[0]
                    if (len(matches) < 1):
                        print "looking for %f-%f GHz inside %f-%f" % (xfrequencies[0],xfrequencies[-1],fa[0],fa[-1])
                    amplitudeSolutionX = np.mean(gampx)*(cc-np.mean(cc)+1)

                    cc = calcChebyshev(polynomialAmplitude[index][nPolyAmp[index]:2*nPolyAmp[index]], validDomain, frequenciesGHz[index]*1e+9)
                    if (nPolarizations > 1):
                      if (yfrequencies[0] < yfrequencies[-1]):
                        matches = np.where(fa>yfrequencies[0])[0]
                        matches2 = np.where(fa<yfrequencies[-1])[0]
                      else:
                        matches = np.where(fa>yfrequencies[-1])[0]
                        matches2 = np.where(fa<yfrequencies[0])[0]
                      amplitudeSolutionY = np.mean(gampy)*(cc-np.mean(cc)+1)
                    if (bpolyOverlay2):
                        validDomain = [frequencyLimits2[0,index], frequencyLimits2[1,index]]
                        cc = calcChebyshev(polynomialAmplitude2[index][0:nPolyAmp2[index]],
                                           validDomain, frequenciesGHz2[index]*1e+9)
                        fa = np.array(frequenciesGHz2[index])
                        if (xfrequencies[0] < xfrequencies[-1]):
                            matches = np.where(fa>xfrequencies[0])[0]
                            matches2 = np.where(fa<xfrequencies[-1])[0]
                        else:
                            matches = np.where(fa>xfrequencies[-1])[0]
                            matches2 = np.where(fa<xfrequencies[0])[0]
                        amplitudeSolution2X = np.mean(gampx)*(cc-np.mean(cc)+1)

                        cc = calcChebyshev(polynomialAmplitude2[index][nPolyAmp2[index]:2*nPolyAmp2[index]],
                                           validDomain, frequenciesGHz2[index]*1e+9)
                        fa = np.array(frequenciesGHz2[index])
                        if (yfrequencies[0] < yfrequencies[-1]):
                            matches = np.where(fa>yfrequencies[0])[0]
                            matches2 = np.where(fa<yfrequencies[-1])[0]
                        else:
                            matches = np.where(fa>yfrequencies[-1])[0]
                            matches2 = np.where(fa<yfrequencies[0])[0]
                        amplitudeSolution2Y = np.mean(gampy)*(cc-np.mean(cc)+1)

                        pb.hold(True)
                        for p in range(nPolarizations):
                              if (corrTypeToString(corr_type[p]) in polsToPlot):
                                    pb.plot(pfrequencies[p], gamp[p],'%s%s'%(pcolor[p],ampmarkstyle), markersize=markersize)
                                    newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                        if (corrTypeToString(corr_type[0]) in polsToPlot):
                              pb.plot(frequenciesGHz[index], amplitudeSolutionX,'%s%s'%(p2color[0],bpolymarkstyle))
                              pb.plot(frequenciesGHz2[index], amplitudeSolution2X, '%s%s'%(p3color[0],bpolymarkstyle))
                        if (nPolarizations == 2):
                           if (corrTypeToString(corr_type[1]) in polsToPlot):
                              pb.plot(frequenciesGHz[index], amplitudeSolutionY,'%s%s'%(p2color[1],bpolymarkstyle))
                              pb.plot(frequenciesGHz2[index], amplitudeSolution2Y, '%s%s'%(p3color[1],bpolymarkstyle))
                    else:
                        pb.hold(True)
                        for p in range(nPolarizations):
                              if (corrTypeToString(corr_type[p]) in polsToPlot):
                                    pb.plot(pfrequencies[p], gamp[p],'%s%s'%(pcolor[p],ampmarkstyle), markersize=markersize)
                                    newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                        if (corrTypeToString(corr_type[0]) in polsToPlot):
                              pb.plot(frequenciesGHz[index], amplitudeSolutionX,'%s%s'%(p2color[0],bpolymarkstyle))
                        if (nPolarizations == 2):
                           if (corrTypeToString(corr_type[1]) in polsToPlot):
                              pb.plot(frequenciesGHz[index], amplitudeSolutionY,'%s%s'%(p2color[1],bpolymarkstyle))
                    # endif (bpolyOverlay2)
                else:
                    # we are not overlaying any B or polynomial solutions      'amp vs. freq'
                    if (showflagged):
                        # Also show the flagged data to see where the flags are
                        pb.hold(True)
                        for p in range(nPolarizations):
                          if (corrTypeToString(corr_type[p]) in polsToPlot):
                            if (overlayAntennas or overlayTimes):
                              pdesc1 = pb.plot(pfrequencies[p], gamp[p], '%s'%ampmarkstyles[p], markersize=markersize)
                              newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                              pdesc2 = pb.plot(flagfrequencies[p], pflag[p], '%s'%ampmarkstyles[p],markersize=markersize)
                              newylimits = recalcYlimitsFreq(chanrange, newylimits, pflag[p], sideband,plotrange,xchannels)
                              if (overlayAntennas):
                                  pb.setp(pdesc1, color=overlayColors[xctr])
                                  pb.setp(pdesc2, color=overlayColors[xctr])
                              else: # overlayTimes
                                  if (myUniqueTime != []):
                                      pb.setp(pdesc1, color=overlayColors[myUniqueTime])
                                      pb.setp(pdesc2, color=overlayColors[myUniqueTime])
                                      if (p==0 or len(polsToPlot)==1): myUniqueColor.append(overlayColors[len(myUniqueColor)])
                                      pb.setp(pdesc1, color=myUniqueColor[-1])
                                      pb.setp(pdesc2, color=myUniqueColor[-1])
                            else:
                              pb.plot(pfrequencies[p], gamp[p], '%s%s'%(pcolor[p],ampmarkstyles[p]), markersize=markersize)
                              newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                              pb.plot(flagfrequencies[p], pflag[p], '%s%s'%(p2color[p],ampmarkstyles[p]), markersize=markersize)
                              newylimits = recalcYlimitsFreq(chanrange, newylimits, pflag[p], sideband,plotrange,xchannels)
                    else:   # showing only unflagged data    'amp vs. freq'
                        pb.hold(True)
                        for p in range(nPolarizations):
                          if (corrTypeToString(corr_type[p]) in polsToPlot):
                            if (len(gamp[p]) == 0):  # Try this on Apr 2, 2012
#                                print "=============== Skipping flagged data on antenna %d = %s" % (xant,antstring)
                                continue
                            if (overlayAntennas or overlayTimes):
#                              print "freq: drawing overlay ----------------"
                              pdesc = pb.plot(pfrequencies[p], gamp[p], '%s'%ampmarkstyles[p], markersize=markersize)
                              newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                              if (overlayAntennas):
                                  pb.setp(pdesc, color=overlayColors[xctr])
                              else: # overlayTimes
                                  if (myUniqueTime != []):
                                      pb.setp(pdesc, color=overlayColors[myUniqueTime])
                                      if (p==0 or len(polsToPlot)==1): myUniqueColor.append(overlayColors[len(myUniqueColor)])
                                      pb.setp(pdesc, color=myUniqueColor[-1])
                            else:
                               if (corrTypeToString(corr_type[p]) in polsToPlot):
                                  # since there is no overlay, don't use dashed line, so zero ------v
                                  pb.plot(pfrequencies[p], gamp[p], '%s%s'%(pcolor[p],ampmarkstyles[0]),markersize=markersize)
                                  newylimits = recalcYlimitsFreq(chanrange, newylimits, gamp[p], sideband,plotrange,xchannels)
                        if (sum(xflag)>0):
#                            print "amp: Resetting xaxis frequency range to counteract flagged data"
                            xrange = np.max(frequencies)-np.min(frequencies)
                            SetNewXLimits([np.min(frequencies)-0.15*xrange, np.max(frequencies)+0.15*xrange])
                            
                if (1==1 or (xframe in bottomRowFrames) or (xctr+1==len(antennasToPlot) and ispw==spwsToPlot[-1])):
                    # use 1==1 because spw might change between top row and bottom row of frames
                    pb.xlabel(xlabelString, size=mysize)
            # endif (xaxis=='chan' elif xaxis=='freq'  for 'amp')
            if (overlayTimes):
                timeString =''
            else:
                if (len(uniqueTimes) > mytime):
                    timeString = ',  t%d/%d  %s' % (mytime+1,nUniqueTimes,utstring(uniqueTimes[mytime],3))
            if (ispw==originalSpw[ispw]):
                if (overlayTimes and len(fieldsToPlot) > 1):
                      indices = fstring = ''
                      for f in fieldIndicesToPlot:
                          if (f != fieldIndicesToPlot[0]):
                              indices += ','
                              fstring += ','
                          indices += str(uniqueFields[f])
                          if (msFound):
                              fstring += msFields[uniqueFields[f]]
                      if (len(fstring) > fstringLimit):
                            fstring = fstring[0:fstringLimit] + '...'
                      titleString = "%sspw%d,  fields %s: %s%s" % (antennaString,ispw,
                                indices, fstring, timeString)
                else:
                      titleString = "%sspw%d,  field %d: %s%s" % (antennaString,ispw,uniqueFields[fieldIndex],fieldString,timeString)
            else:
                if (overlayTimes and len(fieldsToPlot) > 1):
                    indices = fstring = ''
                    for f in fieldIndicesToPlot:
                        if (f != fieldIndicesToPlot[0]):
                            indices += ','
                            fstring += ','
                        indices += str(uniqueFields[f])
                        if (msFound):
                            fstring += msFields[uniqueFields[f]]
                    if (len(fstring) > fstringLimit):
                            fstring = fstring[0:fstringLimit] + '...'
                    titleString = "%sspw%d (%d),  fields %s: %s%s" % (antennaString,ispw,originalSpw[ispw],
                                indices, fstring, timeString)
                else:
                    titleString = "%sspw%d (%d),  field %d: %s%s" % (antennaString,ispw,originalSpw[ispw],uniqueFields[fieldIndex],fieldString,timeString)
            pb.title(titleString, size=titlesize)
            if (abs(plotrange[0]) > 0 or abs(plotrange[1]) > 0):
                SetNewXLimits([plotrange[0],plotrange[1]])
            else:
                # Here is 1st place where we eliminate white space on right and left edge of the plots: 'amp'
                if (xaxis.find('chan')>=0):
                    SetNewXLimits([channels[0],channels[-1]])
                else:
                    if (zoom != 'intersect'):
                        SetNewXLimits([frequencies[0], frequencies[-1]])
                    if (bOverlay):
                        if (xrange2 > xrange+0.1 and zoom != 'intersect'):
                            TDMisSecond = True
            if (abs(plotrange[2]) > 0 or abs(plotrange[3]) > 0):
                SetNewYLimits([plotrange[2],plotrange[3]])

            ResizeFonts(adesc,mysize)
            adesc.xaxis.grid(True,which='major')
            adesc.yaxis.grid(True,which='major')
            pb.ylabel(yAmplitudeLabel, size=mysize)
            pb.subplots_adjust(hspace=myhspace, wspace=mywspace)
            xlim = pb.xlim()
            ylim = pb.ylim()
            xrange = xlim[1]-xlim[0]
            yrange = ylim[1]-ylim[0]
            if (overlayAntennas == False and overlayTimes == False and bOverlay == False):
                # draw polarization labels for no overlay
                x0 = xstartPolLabel
                y0 = ystartPolLabel
                for p in range(nPolarizations):
                   if (corrTypeToString(corr_type[p]) in polsToPlot):
                      pb.text(x0, y0-subplotRows*p*0.03, corrTypeToString(corr_type[p]),
                              color=pcolor[p],size=mysize, transform=pb.gca().transAxes)
                if (xframe == firstFrame):
                      # draw title including caltable name
                      caltableList = caltable 
                      if (bpolyOverlay):
                            caltableList += ', ' + caltable2 + ' (degamp=%d, degphase=%d)'%(nPolyAmp[index]-1,nPolyPhase[index]-1)
                            if (bpolyOverlay2):
                                  caltableList += ', ' + caltable3 + ' (degamp=%d, degphase=%d)'%(nPolyAmp2[index]-1,nPolyPhase2[index]-1)
                      pb.text(xstartTitle, ystartTitle, caltableList, size=titlesize,
                              color='k', transform=pb.gcf().transFigure)

            elif (overlayAntennas==True and xant==antennasToPlot[-1] and bOverlay == False):
                    # We do this last, because by then, the limits will be stable.
                    x0 = xstartPolLabel
                    y0 = ystartPolLabel
                    # draw polarization labels
                    if (corrTypeToString(corr_type[0]) in polsToPlot):
                      if (ampmarkstyle.find('-')>=0):
                          pb.text(x0, y0, corrTypeToString(corr_type[0])+' solid', color=overlayColors[0],size=mysize,
                                  transform=pb.gca().transAxes)
                      else:
                          pb.text(x0+0.02, y0, corrTypeToString(corr_type[0]), color=overlayColors[0],size=mysize,
                                  transform=pb.gca().transAxes)
                          pdesc = pb.plot([x0-0.01], [y0], '%sk'%ampmarkstyle, markersize=markersize,
                                          scalex=False,scaley=False, transform=pb.gca().transAxes)
                    if (len(corr_type) > 1):
                     if (corrTypeToString(corr_type[1]) in polsToPlot):
                      if (ampmarkstyle2.find('--')>=0):
                        pb.text(x0, y0-0.03*subplotRows, corrTypeToString(corr_type[1])+' dashed',
                                color=overlayColors[0],size=mysize, transform=pb.gca().transAxes)
                      else:
                        pb.text(x0+0.02, y0-0.03*subplotRows, corrTypeToString(corr_type[1]),
                                color=overlayColors[0],size=mysize, transform=pb.gca().transAxes)
                        pdesc = pb.plot([x0-0.01], [y0-0.03*subplotRows], '%sk'%ampmarkstyle2,
                                        markersize=markersize, scalex=False,scaley=False)
                    if (xframe == firstFrame):
                        # draw title including caltable name
                        pb.text(xstartTitle, ystartTitle, caltable, size=titlesize, color='k',
                                transform=pb.gcf().transFigure)
                        DrawAntennaNames(msAnt, antennasToPlot, msFound, mysize)
            elif (overlayTimes==True and bOverlay == False):
                doneTimeOverlay = True
                for f in fieldIndicesToPlot:
                  if (len(uniqueTimesPerFieldPerSpw[ispwInCalTable][f]) > 0):
                    if ((uniqueTimes[mytime] < uniqueTimesPerFieldPerSpw[ispwInCalTable][f][-1]-solutionTimeThresholdSeconds) and
                        (uniqueTimes[mytime] < timerangeListTimes[-1])):
#                        print "Not done because %.0f < %.0f for fieldIndex=%d" % (uniqueTimes[mytime], uniqueTimesPerFieldPerSpw[ispwInCalTable][f][-1],f)
                        doneTimeOverlay = False
                if (doneTimeOverlay):
                # either it is the last time of any times in solution, or the last time in the list of times to plot
#                    print "*** on last time = %d for last fieldIndex %d  or %d>=%d" % (mytime,fieldIndex,mytime,timerangeList[-1])
                    mytime = nUniqueTimes-1
                    # We do this last, because by then, the limits will be broad enought and stable.
                    # draw polarization labels
                    x0 = xstartPolLabel
                    y0 = ystartPolLabel
                    if (corrTypeToString(corr_type[0]) in polsToPlot):
                        if (ampmarkstyle.find('-')>=0):
                            pb.text(x0, y0, corrTypeToString(corr_type[0])+' solid', color='k',
                                    size=mysize, transform=pb.gca().transAxes)
                        else:
                            pb.text(x0+0.02, y0, corrTypeToString(corr_type[0]), color='k',
                                    size=mysize, transform=pb.gca().transAxes)
                            pdesc = pb.plot([x0-0.1], [y0], '%sk'%ampmarkstyle, markersize=markersize,
                                            scalex=False,scaley=False, transform=pb.gca().transAxes)
                    if (len(corr_type) > 1):
                     if (corrTypeToString(corr_type[1]) in polsToPlot):
                        if (ampmarkstyle2.find('--')>=0):
                            pb.text(x0, y0-0.03*subplotRows, corrTypeToString(corr_type[1])+' dashed',
                                    color='k', size=mysize, transform=pb.gca().transAxes)
                        else:
                            pb.text(x0+0.02*xrange, y0-0.03*subplotRows, corrTypeToString(corr_type[1]),
                                    color='k', size=mysize, transform=pb.gca().transAxes)
                            pdesc = pb.plot([x0-0.1], [y0-0.03*subplotRows], '%sk'%ampmarkstyle2,
                                            markersize=markersize, scalex=False,scaley=False, transform=pb.gca().transAxes)
                    if (xframe == firstFrame):
                        # draw title including caltable name
                        pb.text(xstartTitle, ystartTitle, caltable, size=titlesize,
                                color='k', transform=pb.gcf().transFigure)
                        if (multiFieldsWithOverlayTime):
                          # support multi-fields with overlay='time'
                          uTPFPS = []
                          for f in fieldIndicesToPlot:
                              for t in uniqueTimesPerFieldPerSpw[ispwInCalTable][f]:
                                  if (sloppyMatch(t, timerangeListTimes, solutionTimeThresholdSeconds,
                                                  myprint=debugSloppyMatch)):
                                      uTPFPS.append(t)
                          uTPFPS = np.sort(uTPFPS)
                          for a in range(len(uTPFPS)):
                            legendString = utstring(uTPFPS[a],220)
#                            print "Drawing %s" % (legendString)
                            if (a==0):
                                pb.text(xstartTitle-0.02, ystartOverlayLegend, 'UT',color='k',fontsize=mysize,
                                        transform=pb.gcf().transFigure)
                            if (a < maxTimesAcrossTheTop):
                                x0 = xstartTitle + (a*timeHorizontalSpacing)
                                y0 = ystartOverlayLegend
                            else:
                                # start going down the righthand side
                                x0 = xstartTitle + (maxTimesAcrossTheTop*timeHorizontalSpacing)
                                y0 = ystartOverlayLegend-(a-maxTimesAcrossTheTop)*antennaVerticalSpacing
                            for tlt in timerangeListTimes:
                                 if (sloppyMatch(uTPFPS[a],timerangeListTimes,solutionTimeThresholdSeconds,debugSloppyMatch)):
                                     myUniqueTime = uTPFPS[a]
                            if (len(fieldsToPlot) > 1 or len(timerangeList) > 1):
#                                print "len(uTPFPS)=%d, a=%d, len(myUniqueColor)=%d" % (len(uTPFPS),a,len(myUniqueColor))
                                # stopgap until I understand why it gets off by one in I16293 Band9 data
                                if (a >= len(myUniqueColor)):
                                    myUniqueColor.append(overlayColors[a])
                                pb.text(x0, y0, legendString,color=myUniqueColor[a],fontsize=mysize,
                                        transform=pb.gcf().transFigure)
                            else:
                                pb.text(x0, y0, legendString,fontsize=mysize, transform=pb.gcf().transFigure)
                        else:
                            for a in range(len(uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex])):
                              legendString = utstring(uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex][a],220)
                              x0 = xstartTitle + (a*timeHorizontalSpacing)
                              y0 = ystartOverlayLegend
                              if (a==0):
                                  pb.text(xstartTitle-0.02, y0, 'UT.',color='k',fontsize=mysize, transform=pb.gcf().transFigure)
                              pb.text(x0,y0, legendString,color=overlayColors[a],fontsize=mysize,
                                      transform=pb.gcf().transFigure)
            else:
                # This will only happen for overlay='antenna,time'
                if (xframe == firstFrame and mytime == 0 and xctr==firstUnflaggedAntennaToPlot):
                    # draw title including caltable name
                    pb.text(xstartTitle, ystartTitle, caltable, size=titlesize, color='k',
                            transform=pb.gcf().transFigure)
                    DrawBottomLegendPageCoords(msName, uniqueTimes[mytime], mysize)

            # Here is 2nd place where we eliminate any white space on the right and left edge of the plots: 'amp'
            # 
            if (abs(plotrange[2]) > 0 or abs(plotrange[3]) > 0):
                SetNewYLimits([plotrange[2],plotrange[3]])
            if (plotrange[0]==0 and plotrange[1]==0):
                if (xaxis.find('chan')>=0):
                    SetNewXLimits([channels[0],channels[-1]])
                else:
                    if (zoom != 'intersect'):
                        SetNewXLimits([frequencies[0], frequencies[-1]])
                    if (bOverlay):
#                        print "Checking if %f >= %f" % (xrange2, xrange)
                        if (xrange2 >= xrange and zoom != 'intersect'):
                            # This is necessary if caltable2=TDM and caltable=FDM
                            SetNewXLimits([frequencies2[0], frequencies2[-1]])
                        if (xrange2 > xrange+0.1 and zoom != 'intersect'):
                            TDMisSecond = True
            else:
                SetNewXLimits([plotrange[0], plotrange[1]])

            # I need the following line for chanrange to work
            if (chanrange[0] != 0 or chanrange[1] != 0):
                SetLimits(plotrange, chanrange, newylimits, channels, frequencies, pfrequencies, ampMin, ampMax, xaxis,pxl)

            # Finally, draw the atmosphere and FDM windows, if requested.
            if ((overlayAntennas==False and overlayTimes==False) or
                    (overlayAntennas==True and xant==antennasToPlot[-1]) or
                    (overlayTimes==True and doneTimeOverlay)):
                if (showatm or showtsky):
                    DrawAtmosphere(showatm, showtsky, subplotRows, atmString,
                                   mysize, TebbSky, plotrange, xaxis, atmchan,
                                   atmfreq, transmission, subplotCols,
                                   showatmPoints=showatmPoints, xframe=xframe)
                    if (LO1 != None):
                        # Now draw the image band
                        DrawAtmosphere(showatm,showtsky, subplotRows, atmString,
                            mysize, TebbSkyImage, plotrange, xaxis,
                            atmchanImage, atmfreqImage, transmissionImage,
                            subplotCols, LO1, xframe, firstFrame, showatmPoints)
                if (xaxis.find('freq')>=0 and showfdm and nChannels <= 256):
                    showFDM(originalSpw, chanFreqGHz)

            if (bOverlay):
                # draw polarization labels
                x0 = xstartPolLabel
                y0 = ystartPolLabel
                for p in range(nPolarizations):
                    if (corrTypeToString(corr_type[p]) in polsToPlot):
                        pb.text(x0, y0-p*0.03*subplotRows, corrTypeToString(corr_type[p])+'-c1',
                                color=pcolor[p],size=mysize,transform=pb.gca().transAxes)
                        pb.text(x0, y0-p*0.03*subplotRows-0.06*subplotRows, corrTypeToString(corr_type[p])+'-c2',
                                color=p2color[p],size=mysize,transform=pb.gca().transAxes)
            if (bpolyOverlay and xaxis.find('freq')>=0):
                # draw polarization labels
                x0 = xstartPolLabel
                y0 = ystartPolLabel
                if (x2color != xcolor):
                      for p in range(nPolarizations):
                          if (corrTypeToString(corr_type[0]) in polsToPlot):
                              pb.text(x0+0.1, y0-p*0.03*subplotRows, corrTypeToString(corr_type[p]), color=p2color[p],
                                      size=mysize,transform=pb.gca().transAxes)
                if (bpolyOverlay2):
                      for p in range(nPolarizations):
                            if (corrTypeToString(corr_type[0]) in polsToPlot):
                                  pb.text(x0+0.2, y0-p*0.03*subplotRows, corrTypeToString(corr_type[p]),
                                          color=p3color[p], size=mysize,transform=pb.gca().transAxes)
                
            if (xframe == 111 and amplitudeWithPhase):
                     if (len(figfile) > 0):
                           plotfiles.append(makeplot(figfile,msFound,msAnt,
                                            overlayAntennas,pages,pagectr,
                                            density,interactive,antennasToPlot,
                                            spwsToPlot,overlayTimes,3))
                     donetime = timeUtilities.time()
                     if (interactive):
                        pb.draw()
#                        myinput = raw_input(":(%.1f sec) Press return for next page (b for backwards, q to quit): "%(donetime-mytimestamp))
                        myinput = raw_input("Press return for next page (b for backwards, q to quit): ")
                     else:
                        myinput = ''
                     mytimestamp = timeUtilities.time()
                     if (myinput.find('q') >= 0):
                         return(vm)
                     if (myinput.find('b') >= 0):
                         if (pagectr > 0):
                             pagectr -= 1
                         #redisplay the current page by setting ctrs back to the value they had at start of that page
                         xctr = pages[pagectr][PAGE_ANT]
                         spwctr = pages[pagectr][PAGE_SPW]
                         mytime = pages[pagectr][PAGE_TIME]
                         myap = pages[pagectr][PAGE_AP]
                         xant = antennasToPlot[xctr]
                         ispw = spwsToPlot[spwctr]
#                         print "Returning to [%d,%d,%d,%d]" % (xctr,spwctr,mytime,myap)
                         redisplay = True
                         if (xctr==pages[0][PAGE_ANT] and spwctr==pages[0][PAGE_SPW] and mytime==pages[0][PAGE_TIME] and pages[0][PAGE_AP]==myap):
                           pb.clf()
                           xframe = xframeStart
                           myUniqueColor = []
                           continue
                     else:
                         pagectr += 1
                         if (pagectr >= len(pages)):
                           pages.append([xctr,spwctr,mytime,1])
#                           print "amp: appending [%d,%d,%d,%d]" % (xctr,spwctr,mytime,1)
                           newpage = 0
                     pb.clf()
                     xframe = xframeStart
                     myUniqueColor = []

######### Here is the phase plotting ############

        if (yaxis.find('phase')>=0 or amplitudeWithPhase):
            if (overlayTimes == False or mytime==firstTimeMatch):  
                if (overlayAntennas==False or xctr==firstUnflaggedAntennaToPlot
                    or xctr>antennasToPlot[-1]):  # 2012-05-24, to fix the case where all ants flagged on one timerange
                    xframe += 1
#                    print "u) incrementing xframe to %d" % xframe
                    myUniqueColor = []
                    newylimits = [LARGE_POSITIVE, LARGE_NEGATIVE]
                    if (phase != ''):
                        if ((phase[0] != 0 or phase[1] != 0) and amplitudeWithPhase):
                            newylimits = phase
            if (debug):
                print "$$$$$$$$$$$$$$$$$$$$$$$  ready to plot phase on xframe %d" % (xframe)
            adesc = pb.subplot(xframe)
            pb.hold(overlayAntennas or overlayTimes)
            gphsx = np.arctan2(np.imag(gplotx),np.real(gplotx))*180.0/math.pi
            if (nPolarizations == 2):
                gphsy = np.arctan2(np.imag(gploty),np.real(gploty))*180.0/math.pi
                gphs = [gphsx,gphsy]
            else:
                gphs = [gphsx]
            if (bOverlay):
                  if (debug):
                      print "computing phase for second table"
                  gphsx2 = np.arctan2(np.imag(gplotx2),np.real(gplotx2))*180.0/math.pi
                  if (nPolarizations == 2):
                        gphsy2 = np.arctan2(np.imag(gploty2),np.real(gploty2))*180.0/math.pi
                        gphs2 = [gphsx2,gphsy2]
                  else:
                        gphs2 = [gphsx2]
            else:
                  if (debug):
                      print "bOverlay is FALSE ==========================="
                
            if (xaxis.find('chan')>=0 or len(xfrequencies) < 1):    # 'phase'
                pb.hold(True)
                for p in range(nPolarizations):
                  if (corrTypeToString(corr_type[p]) in polsToPlot):
                    if (overlayAntennas or overlayTimes):
                        pdesc = pb.plot(pchannels[p],gphs[p],'%s'%(phasemarkstyles[p]),markersize=markersize)
                        newylimits =  recalcYlimits(plotrange,newylimits,gphs[p])  # 10/27/2011
                        if (newylimits[1]-newylimits[0] < minPhaseRange):
                            newylimits = [-minPhaseRange,minPhaseRange]
                        if (phase != ''):
                            if ((phase[0] != 0 or phase[1] != 0) and amplitudeWithPhase):
                                newylimits = phase

                        if (overlayAntennas):
                            pb.setp(pdesc, color=overlayColors[xctr])
                        else: # overlayTimes
                            if (myUniqueTime != []):
                                pb.setp(pdesc, color=overlayColors[myUniqueTime])
                                if (p==0 or len(polsToPlot)==1): myUniqueColor.append(overlayColors[len(myUniqueColor)])
                                pb.setp(pdesc, color=myUniqueColor[-1])
                    else:
                        pb.plot(pchannels[p],gphs[p],'%s%s'%(pcolor[p],phasemarkstyles[0]), markersize=markersize)
                        newylimits =  recalcYlimits(plotrange,newylimits,gphs[p]) # 10/27/2011
                        if (newylimits[1]-newylimits[0] < minPhaseRange):
                            newylimits = [-minPhaseRange,minPhaseRange]
                        if (phase != ''):
                            if ((phase[0] != 0 or phase[1] != 0) and amplitudeWithPhase):
                                newylimits = phase
                if (sum(xflag)>0):
#                    print "phase: Resetting xaxis channel range to counteract flagged data"
                    xrange = np.max(channels)-np.min(channels)
                    SetNewXLimits([np.min(channels)-xrange/20, np.max(channels)+xrange/20])
                if (xframe in bottomRowFrames or (xctr+1==len(antennasToPlot) and ispw==spwsToPlot[-1])):
                    pb.xlabel("Channel", size=mysize)
            elif (xaxis.find('freq')>=0):     # 'phase'
                if (bOverlay):
                      pb.hold(True)
                      if (debug):
                          print "Preparing to plot phase from %f-%f for pols:" % (xfrequencies[0],xfrequencies[-1]),polsToPlot
                          print "Preparing to plot phase from %f-%f for pols:" % (pfrequencies[p][0],pfrequencies[p][-1]),polsToPlot
                          print "Preparing to plot phase from %f-%f for pols:" % (pfrequencies2[p][0],pfrequencies2[p][-1]),polsToPlot
                      xrange = np.abs(xfrequencies[0]-xfrequencies[-1])
                      try:
                          xrange2 = np.abs(xfrequencies2[0]-xfrequencies2[-1])
                      except:
                          print "No phase data found in second solution.  Try increasing the solutionTimeThresholdSeconds above %.0f." % (solutionTimeThresholdSeconds)
                          print "If this doesn't work, email the developer (%s)." % (developerEmail)
                          return(vm)
                      if (np.abs(xrange/xrange2 - 1) > 0.05 + len(xflag)/len(xchannels)):  # 0.0666 is 2000/1875-1
                         # These line widths are optimal for visualizing FDM over TDM
                         width1 = 1
                         width2 = 4
                         # solutions differ in frequency width, so show the narrower one first
                         if (xrange < xrange2):
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                                if (debug): print "pb.plot 1"
                                pb.plot(pfrequencies[p], gphs[p], '%s%s'%(pcolor[p],phasemarkstyle), linewidth=width2)
                                newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs[p], sideband,plotrange,xchannels)
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                                if (debug): print "pb.plot 2"
                                pb.plot(pfrequencies2[p], gphs2[p], '%s%s'%(p2color[p],phasemarkstyle), linewidth=width1)
                                newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs2[p], sideband,plotrange,xchannels2)
                         else:
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                                 if (debug): print "pb.plot 3"
                                 pb.plot(pfrequencies2[p], gphs2[p], '%s%s'%(p2color[p],phasemarkstyle), linewidth=width2)
                                 newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs2[p], sideband,plotrange,xchannels2)
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                                 if (debug): print "pb.plot 4"
                                 pb.plot(pfrequencies[p], gphs[p], '%s%s'%(pcolor[p],phasemarkstyle), linewidth=width1)
                                 newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs[p], sideband,plotrange,xchannels)
                      else:
                         width1 = 1
                         width2 = 1
                         # solutions may be different level of smoothing, so plot highest rms first
                         pb.hold(True)
                         if (np.std(gphsx) < np.std(gphsx2)):
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                               if (debug): print "pb.plot 5"
                               pb.plot(pfrequencies2[p], gphs2[p], '%s%s'%(p2color[p],phasemarkstyle), linewidth=width1)
                               newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs2[p], sideband,plotrange,xchannels2)
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                               if (debug): print "pb.plot 6"
                               pb.plot(pfrequencies[p], gphs[p], '%s%s'%(pcolor[p],phasemarkstyle), linewidth=width2)
                               newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs[p], sideband,plotrange,xchannels)
                         else:
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                               if (debug): print "pb.plot 7"
                               pb.plot(pfrequencies[p], gphs[p], '%s%s'%(pcolor[p],phasemarkstyle), linewidth=width2)
                               newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs[p], sideband,plotrange,xchannels)
                           for p in range(nPolarizations):
                             if (corrTypeToString(corr_type[p]) in polsToPlot):
                               if (debug): print "pb.plot 9"
                               pb.plot(pfrequencies2[p], gphs2[p], '%s%s'%(p2color[p],phasemarkstyle), linewidth=width1)
                               newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs2[p], sideband,plotrange,xchannels2)
                      # must set new limits after plotting  'phase'
                      (y0,y1) = pb.ylim()
                      if (y1-y0 < minPhaseRange):
                            # this must come before defining ticks 
                            SetNewYLimits([-minPhaseRange,minPhaseRange])
                      if (zoom=='intersect'):
                          if (xrange < xrange2):
                              SetNewXLimits([min(xfrequencies[0],xfrequencies[-1])-xrange*0.1, max(xfrequencies[0],xfrequencies[-1])+xrange*0.1])
                              SetLimits(plotrange, chanrange, newylimits, channels, frequencies,
                                        pfrequencies, ampMin, ampMax, xaxis,pxl)
                          else:
                              SetNewXLimits([min(xfrequencies2[0],xfrequencies2[-1])-xrange2*0.1, max(xfrequencies2[0],xfrequencies2[-1])+xrange2*0.1])
                              SetLimits(plotrange, chanrange, newylimits, channels, frequencies2,
                                        pfrequencies2, ampMin, ampMax, xaxis,pxl)
                      else:
                          if (xrange < xrange2):
                              SetLimits(plotrange, chanrange, newylimits, channels, frequencies,
                                        pfrequencies, ampMin, ampMax, xaxis,pxl)
                          else:
                              SetLimits(plotrange, chanrange, newylimits, channels, frequencies2,
                                        pfrequencies2, ampMin, ampMax, xaxis,pxl)
                      # draw polarization and spw labels
                      if (xframe == firstFrame):
                          # draw title including caltable name
                          caltableList = 'c1 = ' + caltable + ',  c2 = ' + caltable2 + ' (%s)'%(utstring(uniqueTimes2[mytime],3))
                          pb.text(xstartTitle, ystartTitle, caltableList, size=titlesize,
                                  color='k', transform=pb.gcf().transFigure)
                elif (bpolyOverlay):
                        matches1 = []
                        for tbp in range(len(timesBP)):
                            if (sloppyMatch(uniqueTimes[mytime], timesBP[tbp], solutionTimeThresholdSeconds,myprint=debugSloppyMatch)):
                                matches1.append(tbp)
                        matches1 = np.array(matches1)
                        if (len(matches1) < 1):
                            print "No time match found"
                            print "If you are sure the solutions correspond to the same data, you can set solutionTimeThresholdSeconds=%.0f" % (1+np.ceil(np.abs(timesBP[0]-uniqueTimes[mytime])))
                            return(vm)
#                        matches1 = np.where(np.floor(uniqueTimes[mytime]) == np.floor(np.array(timesBP)))[0]
                        matches2 = np.where(xant == np.array(antennasBP))[0]
                        if (len(matches2) < 1):
                            print "No antenna match found: ", xant, antennasBP
                        matches3 = np.where(ispw == np.array(cal_desc_idBP))[0]
                        if (len(matches3) < 1):
                            print "No spw match found: ", ispw, cal_desc_idBP
                        matches12 = np.intersect1d(matches1,matches2)
                        if (len(matches12) < 1):
                            print "No match between: ", matches1, matches2
                        matches = np.intersect1d(matches12, matches3)
                        if (len(matches) < 1):
                            print "No match between: ", matches12, matches3
                        try:
                            index = matches[0]
                        except:
                            print "No match found for time=%.6f, xant=%d, ispw=%d"  % (uniqueTimes[mytime],xant,ispw)
                            print "antennasBP = ", antennasBP
                            print "cal_desc_idBP = ", cal_desc_idBP
                            print "timesBP = "
                            for i in timesBP:
                                print "%.6f, " % i
                            return(vm)
#                        print "phase: Using index = %d/%d (mytime=%d), domain=%.3f,%.3f" % (index,len(polynomialPhase),mytime,frequencyLimits[0,index]*1e-9,frequencyLimits[1,index]*1e-9)
                        validDomain = [frequencyLimits[0,index], frequencyLimits[1,index]]
                        cc = calcChebyshev(polynomialPhase[index][0:nPolyPhase[index]], validDomain, frequenciesGHz[index]*1e+9) * 180/math.pi
                        fa = np.array(frequenciesGHz[index])
                        if (xfrequencies[0] < xfrequencies[-1]):
                            matches = np.where(fa>xfrequencies[0])[0]
                            matches2 = np.where(fa<xfrequencies[-1])[0]
                        else:
                            matches = np.where(fa>xfrequencies[-1])[0]
                            matches2 = np.where(fa<xfrequencies[0])[0]
                        mymean = complexMeanDeg(np.array(cc)[matches[0]:matches2[-1]+1])
                        phaseSolutionX = np.mean(gphsx) - mymean + cc

                        cc = calcChebyshev(polynomialPhase[index][nPolyPhase[index]:2*nPolyPhase[index]], validDomain, frequenciesGHz[index]*1e+9) * 180/math.pi
                        if (nPolarizations > 1):
                          if (yfrequencies[0] < yfrequencies[-1]):
                            matches = np.where(fa>yfrequencies[0])[0]
                            matches2 = np.where(fa<yfrequencies[-1])[0]
                          else:
                            matches = np.where(fa>yfrequencies[-1])[0]
                            matches2 = np.where(fa<yfrequencies[0])[0]
                          mymean = complexMeanDeg(np.array(cc)[matches[0]:matches2[-1]+1])
                          phaseSolutionY = np.mean(gphsy) - mymean + cc
                        if (bpolyOverlay2):
                            validDomain = [frequencyLimits2[0,index], frequencyLimits2[1,index]]
                            cc = calcChebyshev(polynomialPhase2[index][0:nPolyPhase2[index]], validDomain, frequenciesGHz2[index]*1e+9) * 180/math.pi
                            fa = np.array(frequenciesGHz2[index])
                            if (xfrequencies[0] < xfrequencies[-1]):
                                matches = np.where(fa>xfrequencies[0])[0]
                                matches2 = np.where(fa<xfrequencies[-1])[0]
                            else:
                                matches = np.where(fa>xfrequencies[-1])[0]
                                matches2 = np.where(fa<xfrequencies[0])[0]
                            mymean = complexMeanDeg(np.array(cc)[matches[0]:matches2[-1]+1])
                            phaseSolution2X = np.mean(gphsx) + cc - mymean

                            cc = calcChebyshev(polynomialPhase2[index][nPolyPhase2[index]:2*nPolyPhase2[index]], validDomain, frequenciesGHz2[index]*1e+9) * 180/math.pi
                            if (yfrequencies[0] < yfrequencies[-1]):
                                matches = np.where(fa>yfrequencies[0])[0]
                                matches2 = np.where(fa<yfrequencies[-1])[0]
                            else:
                                matches = np.where(fa>yfrequencies[-1])[0]
                                matches2 = np.where(fa<yfrequencies[0])[0]
                            mymean = complexMeanDeg(np.array(cc)[matches[0]:matches2[-1]+1])
                            phaseSolution2Y = np.mean(gphsy) + cc - mymean
                            pb.hold(True)
                            for p in range(nPolarizations):
                                  if (corrTypeToString(corr_type[p]) in polsToPlot):
                                        pb.plot(pfrequencies[p], gphs[p],'%s%s'%(pcolor[p],phasemarkstyle), markersize=markersize)
                            if (corrTypeToString(corr_type[0]) in polsToPlot):
                                 pb.plot(frequenciesGHz[index],phaseSolutionX,'%s%s'%(x2color,bpolymarkstyle))
                                 pb.plot(frequenciesGHz2[index],phaseSolution2X,'%s%s'%(x3color,bpolymarkstyle))
                            if (nPolarizations == 2):
                               if (corrTypeToString(corr_type[1]) in polsToPlot):
                                  pb.plot(frequenciesGHz[index],phaseSolutionY,'%s%s'%(y2color,bpolymarkstyle))
                                  pb.plot(frequenciesGHz2[index],phaseSolution2Y,'%s%s'%(y3color,bpolymarkstyle))
                        else:
                            pb.hold(True)
                            for p in range(nPolarizations):
                                  if (corrTypeToString(corr_type[p]) in polsToPlot):
                                        pb.plot(pfrequencies[p], gphs[p],'%s%s'%(pcolor[p],phasemarkstyle), markersize=markersize)
                            if (corrTypeToString(corr_type[0]) in polsToPlot):
                               pb.plot(frequenciesGHz[index],phaseSolutionX,'%s%s'%(x2color,bpolymarkstyle))
                            if (nPolarizations == 2):
                               if (corrTypeToString(corr_type[1]) in polsToPlot):
                                  pb.plot(frequenciesGHz[index],phaseSolutionY,'%s%s'%(y2color,bpolymarkstyle))
                        # endif (bpolyOverlay2)
                else:
                    # we are not overlaying any B or polynomial solutions   'phase vs. freq'
                    pb.hold(True)
                    for p in range(nPolarizations):
                        if (corrTypeToString(corr_type[p]) in polsToPlot):
                            if (overlayAntennas or overlayTimes):
                              pdesc = pb.plot(pfrequencies[p], gphs[p],'%s'%(phasemarkstyles[p]), markersize=markersize)
                              newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs[p], sideband,plotrange,xchannels) # Apr 2, 2012
                              if (overlayAntennas):
                                  pb.setp(pdesc, color=overlayColors[xctr])
                              else: # overlayTimes
                                  if (myUniqueTime != []):
                                      pb.setp(pdesc, color=overlayColors[myUniqueTime])
                                      if (p==0 or len(polsToPlot)==1): myUniqueColor.append(overlayColors[len(myUniqueColor)])
                                      pb.setp(pdesc, color=myUniqueColor[-1])
                            else:
                              pb.plot(pfrequencies[p], gphs[p],'%s%s'%(pcolor[p],phasemarkstyles[0]), markersize=markersize)
                              newylimits = recalcYlimitsFreq(chanrange, newylimits, gphs[p], sideband, plotrange,xchannels)
                        if (sum(xflag)>0):
#                            print "phase frame %d: Resetting xaxis frequency range to counteract flagged data" % (xframe)
                            xrange = np.max(frequencies)-np.min(frequencies)
                            SetNewXLimits([np.min(frequencies)-0.15*xrange, np.max(frequencies)+0.15*xrange])
                        if (np.max(gphs[p]) < minPhaseRange and np.min(gphs[p]) > -minPhaseRange):
                            SetNewYLimits([-minPhaseRange,minPhaseRange])
                #endif bOverlay

                if (1==1):
                    pb.xlabel(xlabelString, size=mysize)
            #endif xaxis='chan'/freq  for 'phase'
            if (overlayTimes):
                timeString =''
            else:
                timeString = ',  t%d/%d  %s' % (mytime+1,nUniqueTimes,utstring(uniqueTimes[mytime],3))
            if (ispw==originalSpw[ispw]):
                  titleString = "%sspw%d,  field %d: %s%s" % (antennaString,ispw,
                                                              uniqueFields[fieldIndex],fieldString,timeString)
            else:
                  titleString = "%sspw%d (%d),  field %d: %s%s" % (antennaString,ispw,originalSpw[ispw],
                                                              uniqueFields[fieldIndex],fieldString,timeString)
            pb.title(titleString,size=titlesize)
            if (abs(plotrange[0]) > 0 or abs(plotrange[1]) > 0):
                SetNewXLimits([plotrange[0],plotrange[1]])

            # Here is 1st place where we eliminate any white space on the right and left edge of the plots: 'phase'
            else:
                if (xaxis.find('chan')>=0):
                    SetNewXLimits([channels[0],channels[-1]])
                else:
                    if (zoom != 'intersect'):
                        SetNewXLimits([frequencies[0], frequencies[-1]])
                    if (bOverlay):
                        if (xrange2 > xrange+0.1 and zoom != 'intersect'):
                            TDMisSecond = True

            if (abs(plotrange[2]) > 0 or abs(plotrange[3]) > 0):
                if (amplitudeWithPhase == False or phase == ''):
                    SetNewYLimits([plotrange[2],plotrange[3]])
            if (amplitudeWithPhase and phase != ''):
                if (phase[0] != 0 or phase[1] != 0):
                    SetNewYLimits(phase)
                
            (y0,y1) = pb.ylim()
            if (y1-y0 < minPhaseRange):
                  # this must come before defining ticks
                  SetNewYLimits([-minPhaseRange,minPhaseRange])
            if (amplitudeWithPhase and phase != ''):
                if (phase[0] != 0 or phase[1] != 0):
                    SetNewYLimits(phase)
            (y0,y1) = pb.ylim()
            ResizeFonts(adesc,mysize)
            adesc.xaxis.grid(True,which='major')
            adesc.yaxis.grid(True,which='major')
            pb.ylabel("Phase (deg)", size=mysize)
            pb.subplots_adjust(hspace=myhspace, wspace=mywspace)
            ylim = pb.ylim()
            xlim = pb.xlim()
            xrange = xlim[1]-xlim[0]
            yrange = ylim[1]-ylim[0]
            myap = 0
            if (overlayAntennas == False and overlayTimes == False and bOverlay == False):
                # draw polarization labels
                x0 = xstartPolLabel
                y0 = ystartPolLabel
                for p in range(nPolarizations):
                      if (corrTypeToString(corr_type[p]) in polsToPlot):
                          pb.text(x0, y0-0.03*subplotRows*p, corrTypeToString(corr_type[p]), color=pcolor[p],
                                  size=mysize, transform=pb.gca().transAxes)
                if (xframe == firstFrame):
                      # draw title including caltable name
                      caltableList = caltable
                      if (bpolyOverlay):
                            caltableList += ', ' + caltable2 + ' (degamp=%d, degphase=%d)'%(nPolyAmp[index]-1,nPolyPhase[index]-1)
                            if (bpolyOverlay2):
                                  caltableList += ', ' + caltable3 + ' (degamp=%d, degphase=%d)'%(nPolyAmp2[index]-1,nPolyPhase2[index]-1)
                      pb.text(xstartTitle, ystartTitle, caltableList, size=titlesize,
                              color='k', transform=pb.gcf().transFigure)
            elif (overlayAntennas==True and xant==antennasToPlot[-1] and bOverlay==False):
                # We do this last, because by then, the limits will be stable.
                x0 = xstartPolLabel
                y0 = ystartPolLabel
                if (corrTypeToString(corr_type[0]) in polsToPlot):
                    if (phasemarkstyle.find('-')>=0):
                        pb.text(x0, y0-0.03*subplotRows*0, corrTypeToString(corr_type[0])+' solid', color=overlayColors[0],
                                fontsize=mysize, transform=pb.gca().transAxes)
                    else:
                        pb.text(x0+0.02, y0-0.03*subplotRows*0, corrTypeToString(corr_type[0]), color=overlayColors[0],
                                fontsize=mysize, transform=pb.gca().transAxes)
                        pdesc = pb.plot([x0], [y0+0.015-0*0.03*subplotRows], '%sk'%phasemarkstyle, markersize=markersize,
                                        scalex=False,scaley=False, transform=pb.gca().transAxes)
                if (len(corr_type) > 1):
                  if (corrTypeToString(corr_type[1]) in polsToPlot):
                    if (phasemarkstyle2.find('--')>=0):
                        pb.text(x0, y0-0.03*subplotRows*1, corrTypeToString(corr_type[1])+' dashed', color=overlayColors[0],
                                fontsize=mysize, transform=pb.gca().transAxes)
                    else:
                        pb.text(x0+0.02, y0-0.03*subplotRows*1, corrTypeToString(corr_type[1]), color=overlayColors[0],
                                fontsize=mysize, transform=pb.gca().transAxes)
                        pdesc = pb.plot([x0], [y0+0.015*subplotRows-0.03*subplotRows*1],'%sk'%phasemarkstyle2, markersize=markersize,
                                        scalex=False,scaley=False, transform=pb.gca().transAxes)
                if (xframe == firstFrame):
                    # draw title including caltable name
                    pb.text(xstartTitle, ystartTitle, caltable, size=titlesize, color='k',
                            transform=pb.gcf().transFigure)
                    DrawAntennaNames(msAnt, antennasToPlot, msFound, mysize)
            elif (overlayTimes==True and bOverlay == False):
                doneTimeOverlay = True
                for f in fieldIndicesToPlot:
                    if (uniqueTimes[mytime] < uniqueTimesPerFieldPerSpw[ispwInCalTable][f][-1]-solutionTimeThresholdSeconds and
                        uniqueTimes[mytime] < timerangeListTimes[-1]):
                        doneTimeOverlay = False
                if (doneTimeOverlay):
                    # either it is the last time of any times in solution, or the last time in the list of times to plot
                    mytime = nUniqueTimes-1
                    # We do this last, because by then, the limits will be broad enough and stable.
                    x0 = xstartPolLabel
                    y0 = ystartPolLabel
                    if (corrTypeToString(corr_type[0]) in polsToPlot):
                      if (phasemarkstyle.find('-')>=0):
                          pb.text(x0, y0, corrTypeToString(corr_type[0])+' solid', color='k',
                                  fontsize=mysize, transform=pb.gca().transAxes)
                      else:
                          pb.text(x0+0.02, y0, corrTypeToString(corr_type[0]), color='k',
                                  fontsize=mysize, transform=pb.gca().transAxes)
                          pdesc = pb.plot([x0], [y0+0.015*subplotRows], '%sk'%phasemarkstyle, markersize=markersize,
                                          scalex=False,scaley=False, transform=pb.gca().transAxes)
                    if (len(corr_type) > 1):
                      if (corrTypeToString(corr_type[1]) in polsToPlot):
                        if (phasemarkstyle2.find('--')>=0):
                            pb.text(x0, y0-0.03*subplotRows, corrTypeToString(corr_type[1])+' dashed',
                                    color='k',fontsize=mysize, transform=pb.gca().transAxes)
                        else:
                            pb.text(x0+0.02, y0-0.03*subplotRows, corrTypeToString(corr_type[1]),
                                    color='k', fontsize=mysize, transform=pb.gca().transAxes)
                            pdesc = pb.plot([x0], [y0+0.015*subplotRows-0.03*subplotRows], '%sk'%phasemarkstyle2,
                                            markersize=markersize, scalex=False,scaley=False, transform=pb.gca().transAxes)
                    if (xframe == firstFrame):
                        for a in range(len(uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex])):
                            legendString = utstring(uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex][a],220)
                            if (a==0):
                                pb.text(xstartTitle-0.02, ystartOverlayLegend, 'UT',color='k',fontsize=mysize,
                                        transform=pb.gcf().transFigure)
                            pb.text(xstartTitle+a*timeHorizontalSpacing, ystartOverlayLegend, legendString,
                                    color=overlayColors[a], fontsize=mysize, transform=pb.gcf().transFigure)
            else:
                # This will only happen for overlay='antenna,time'
                if (xframe == firstFrame and mytime==0 and xctr==firstUnflaggedAntennaToPlot):
                    # draw title including caltable name
                    pb.text(xstartTitle, ystartTitle, caltable, size=titlesize, color='k',
                            transform=pb.gcf().transFigure)
                    DrawBottomLegendPageCoords(msName, uniqueTimes[mytime], mysize)
                
            #endif (overlayAntennas == False and overlayTimes == False and bOverlay == False)
            
            # Here is 2nd place where we eliminate any white space on the right and left edge of the plots: 'phase'
            if (abs(plotrange[2]) > 0 or abs(plotrange[3]) > 0):
                if (amplitudeWithPhase == False or phase == ''):
                    SetNewYLimits([plotrange[2],plotrange[3]])
            if (phase != '' and amplitudeWithPhase):
                if (phase[0] != 0 or phase[1] != 0):
                    SetNewYLimits(phase)
            if (plotrange[0]==0 and plotrange[1]==0):
                if (xaxis.find('chan')>=0):
                    SetNewXLimits([channels[0],channels[-1]])
                else:
                    if (zoom != 'intersect'):
                        SetNewXLimits([frequencies[0], frequencies[-1]])
                    if (bOverlay):
                        if (xrange2 >= xrange and zoom != 'intersect'):
                            # This is necessary if caltable2=TDM and caltable=FDM
                            SetNewXLimits([frequencies2[0], frequencies2[-1]])
                        if (xrange2 > xrange+0.1 and zoom != 'intersect'):
                            TDMisSecond = True
            else:
                SetNewXLimits([plotrange[0], plotrange[1]])

            # I need the following line for chanrange to work
            if (chanrange[0] != 0 or chanrange[1] != 0):
                SetLimits(plotrange, chanrange, newylimits, channels, frequencies, pfrequencies, ampMin, ampMax, xaxis,pxl)

            # Finally, draw the atmosphere and FDM windows, if requested.
            if ((overlayAntennas==False and overlayTimes==False) or
                    (overlayAntennas==True and xant==antennasToPlot[-1]) or
                    (overlayTimes==True and doneTimeOverlay)):
                if (showatm or showtsky):
                    DrawAtmosphere(showatm, showtsky, subplotRows, atmString,
                                   mysize, TebbSky, plotrange, xaxis, atmchan,
                                   atmfreq, transmission, subplotCols,
                                   showatmPoints=showatmPoints, xframe=xframe)
                    if (LO1 != None):
                        DrawAtmosphere(showatm,showtsky, subplotRows, atmString,
                                mysize, TebbSky, plotrange, xaxis, atmchanImage,
                                atmfreqImage, transmissionImage, subplotCols,
                                LO1, xframe, firstFrame, showatmPoints)
            
                if (xaxis.find('freq')>=0 and showfdm and nChannels <= 256):
                    showFDM(originalSpw, chanFreqGHz)

            if (bOverlay):
                # draw polarization labels
                x0 = xstartPolLabel
                y0 = ystartPolLabel
                for p in range(nPolarizations):
                    if (corrTypeToString(corr_type[p]) in polsToPlot):
                        pb.text(x0, y0-p*0.03*subplotRows, corrTypeToString(corr_type[p])+'-c1',
                                color=pcolor[p],size=mysize,transform=pb.gca().transAxes)
                        pb.text(x0, y0-(p*0.03+0.06)*subplotRows, corrTypeToString(corr_type[p])+'-c2',
                                color=p2color[p],size=mysize, transform=pb.gca().transAxes)
            if (bpolyOverlay and xaxis.find('freq')>=0):
                x0 = xstartPolLabel
                y0 = ystartPolLabel
                if (xcolor != x2color):
                    for p in range(nPolarizations):
                        if (corrTypeToString(corr_type[p]) in polsToPlot):
                            pb.text(x0+0.1, y0-p*0.03*subplotRows, corrTypeToString(corr_type[p]), color=p2color[p],
                                    size=mysize, transform=pb.gca().transAxes)
                if (bpolyOverlay2):
                    for p in range(nPolarizations):
                          if (corrTypeToString(corr_type[p]) in polsToPlot):
                                pb.text(x0+0.2, y0-p*0.03*subplotRows, corrTypeToString(corr_type[p]), color=p3color[p],
                                        size=mysize, transform=pb.gca().transAxes)

        # endif (yaxis='phase')

        redisplay = False

        if (xframe == lastFrame):
#          print "*** mytime+1 = %d,  nUniqueTimes = %d........" % (mytime+1, nUniqueTimes)
          
          if ((overlayAntennas==False and overlayTimes==False)
              # either it is the last time of any, or the last time in the list of times to plot
              or (overlayAntennas==False and (mytime+1==nUniqueTimes or mytime == timerangeList[-1])) 
              or (xant==antennasToPlot[-1] and overlayAntennas==True)
              or (doneTimeOverlay and overlayTimes==True)):
#              or (overlayTimes==True and sloppyMatch(uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex][-1],uniqueTimes[mytime],solutionTimeThresholdSeconds,myprint=debugSloppyMatch) and fieldIndex==fieldIndicesToPlot[-1])):  # and (xant==antennasToPlot[-1]))):
            
            DrawBottomLegendPageCoords(msName, uniqueTimes[mytime], '8')

            if (len(figfile) > 0):
                  plotfiles.append(makeplot(figfile,msFound,msAnt,
                                            overlayAntennas,pages,pagectr,
                                            density,interactive,antennasToPlot,
                                            spwsToPlot,overlayTimes,4))
            myinput = ''
            donetime = timeUtilities.time()
            if (interactive):
                pb.draw()
#                myinput = raw_input("(%.1f sec) Press return for next screen (b for backwards, q to quit): "%(donetime-mytimestamp))
                myinput = raw_input("Press return for next page (b for backwards, q to quit): ")
            else:
                myinput = ''
            mytimestamp = timeUtilities.time()
            if (myinput.find('q') >= 0):
                mytime = len(uniqueTimes)
                spwctr = len(spwsToPlot)
                xctr = len(antennasToPlot)
                break
            xframe = xframeStart
            myUniqueColor = []
            pb.subplots_adjust(hspace=myhspace, wspace=mywspace)
            if (myinput.find('b') >= 0):
                if (pagectr > 0):
                    pagectr -= 1
                #redisplay the current page by setting ctrs back to the value they had at start of that page
                xctr = pages[pagectr][PAGE_ANT]
                spwctr = pages[pagectr][PAGE_SPW]
                mytime = pages[pagectr][PAGE_TIME]
                myap = pages[pagectr][PAGE_AP]
                xant = antennasToPlot[xctr]
                ispw = spwsToPlot[spwctr]
#                print "Returning to [%d,%d,%d,%d]" % (xctr,spwctr,mytime,myap)
                redisplay = True
            else:
                pagectr += 1
                if (pagectr >= len(pages)):
                    newpage = 1
                else:
                    newpage = 0
            if (overlayTimes==True and sloppyMatch(uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex][-1],uniqueTimes[mytime],solutionTimeThresholdSeconds,myprint=debugSloppyMatch)):
                # be sure to avoid any more loops through mytime which will cause 'b' button to fail
                mytime = nUniqueTimes
#          else:
#              print "Not going to new page, uniqueTimes[mytime]=%.1f, uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex][-1]=%.1f" % (uniqueTimes[mytime],uniqueTimesPerFieldPerSpw[ispwInCalTable][fieldIndex][-1])
        if (redisplay == False):
            if (xctr+1 >= len(antennasToPlot) or overlayAntennas==False): 
                mytime += 1
                if (debug):
                    print "AT BOTTOM OF LOOP: Incrementing mytime to %d, setting firstUnflaggedAntennaToPlot to 0" % (mytime)
                firstUnflaggedAntennaToPlot = 0  # try this
      # end of while(mytime) loop
      if (redisplay == False):
          spwctr += 1
#          print "Incrementing spwctr to %d" % (spwctr)
#      else:
#          print "redisplay = True"
    # end of while(spwctr) loop
    if (xant >= antennasToPlot[-1] and xframe != xframeStart):
        # this is the last antenna, so make a final plot
        if (len(figfile) > 0):
              plotfiles.append(makeplot(figfile,msFound,msAnt,overlayAntennas,
                                     pages,pagectr,density,interactive,
                                     antennasToPlot,spwsToPlot,overlayTimes,5))
    if (redisplay == False):
        xctr += 1
#        print "Incrementing xctr to %d" % (xctr)
    if (overlayAntennas):
#        print "Breaking out of antenna loop because we are done -------------------"
        break
  # end of while(xant) loop
  pb.draw()
  if (len(plotfiles) > 0 and buildpdf):
    pdfname = figfile+'.pdf'
    filelist = ''
    plotfiles = np.unique(plotfiles)
    for i in range(len(plotfiles)):
      cmd = '%s -density %d %s %s.pdf' % (convert,density,plotfiles[i],plotfiles[i].split('.png')[0])
      print "Running command = %s" % (cmd)
      mystatus = os.system(cmd)
      if (mystatus != 0):
          print "ImageMagick's convert command not found, no PDF built"
          buildpdf = False
          break
      filelist += plotfiles[i].split('.png')[0] + '.pdf '
    if (buildpdf):
        cmd = '%s -q -sPAPERSIZE=letter -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s' % (gs,pdfname,filelist)
        print "Running command = %s" % (cmd)
        mystatus = os.system(cmd)
        if (mystatus == 0):
            print "PDF left in %s" % (pdfname)
            os.system("rm -f %s" % filelist)
        else:
            print "ghostscript is missing, no PDF built"
  return(vm)

def calcChebyshev(coeff, validDomain, x):
  """
  Given a set of coefficients,
  this method evaluates a Chebyshev approximation.
  """
  if (type(x) == float or type(x) == int):
       x = [x]
  xrange = validDomain[1] - validDomain[0]
  x = -1 + 2*(x-validDomain[0])/xrange
  coeff[0] = 0
  if (1==1):
    v = np.polynomial.chebval(x,coeff)
  else:
    # manual approach, before I found chebval()
    v = np.zeros(len(x))
    if (len(coeff) > 0):
        v += coeff[0] * 1
    if (len(coeff) > 1):
        v += coeff[1] * (x)
    if (len(coeff) > 2):
        v += coeff[2] * (2*x**2 - 1)
    if (len(coeff) > 3):
        v += coeff[3] * (4*x**3 - 3*x)
    if (len(coeff) > 4):
        v += coeff[4] * (8*x**4 - 8*x**2 + 1)
    if (len(coeff) > 5):
        v += coeff[5] * (16*x**5 - 20*x**3 + 5*x)
    if (len(coeff) > 6):
        v += coeff[6] * (32*x**6 - 48*x**4 + 18*x**2 - 1)
    if (len(coeff) > 7):
        v += coeff[7] * (64*x**7 -112*x**5 + 56*x**3 - 7*x)
    if (len(coeff) > 8):
        v += coeff[8] * (128*x**8 -256*x**6 +160*x**5 - 32*x**2 + 1)
    if (len(coeff) > 9):
        v += coeff[9] * (256*x**9 -576*x**7 +432*x**5 - 120*x**3 + 9*x)
    if (len(coeff) > 10):
        print "Chebyshev polynomials with degree > 10 are not implemented"

  return(v)
    
def ResizeFonts(adesc,fontsize):
#    print "Called ResizeFonts()"
    yFormat = ScalarFormatter(useOffset=False)
    adesc.yaxis.set_major_formatter(yFormat)
    adesc.xaxis.set_major_formatter(yFormat)
    pb.setp(adesc.get_xticklabels(), fontsize=fontsize)
    pb.setp(adesc.get_yticklabels(), fontsize=fontsize)

def complexMeanRad(phases):
    # convert back to real and imaginary, take mean, then convert back to phase
    meanSin = np.mean(np.sin(phases))
    meanCos = np.mean(np.cos(phases))
    return(180*np.arctan2(meanSin, meanCos)/math.pi)

def complexMeanDeg(phases):
    # convert back to real and imaginary, take mean, then convert back to phase
    phases *= math.pi/180
    meanSin = np.mean(np.sin(phases))
    meanCos = np.mean(np.cos(phases))
    return(180*np.arctan2(meanSin, meanCos)/math.pi)

def CalcAtmTransmission(pchan,pfreq,xaxis,pwv,vm,vis,asdm,antenna,timestamp,
                        interval,field,refFreqInTable, net_sideband,
                        mytime, missingCalWVRErrorPrinted,
                        verbose=False):
    telescopeName = au.getObservatoryName(vis)
    if (telescopeName.find('ALMA') >= 0):
        defaultPWV = 1.0   # a reasonable value for ALMA in case it cannot be found
    elif (telescopeName.find('VLA') >= 0):
        defaultPWV = 5.0  
    else:
        defaultPWV = 5.0  
    if (type(pwv) == str):
      if (pwv.find('auto')>=0):
        if (os.path.exists(vis+'/ASDM_CALWVR') or os.path.exists('CalWVR.xml')):
              if (verbose):
                  print "*** Computing atmospheric transmission using measured PWV, field %d, time %d (%f). ***" % (field,mytime,timestamp)
              timerange = [timestamp-interval/2, timestamp+interval/2]
              if (os.path.exists(vis+'/ASDM_CALWVR')):
                  [pwvmean, pwvstd]  = au.getMedianPWV(vis,timerange,asdm,verbose=False)
              else:
                  [pwvmean, pwvstd]  = au.getMedianPWV('.',timerange,asdm='',verbose=False)
              if (verbose):
                  print "retrieved pwvmean = %f" % pwvmean
              retrievedPWV = pwvmean
              if (pwvmean < 0.00001):
                  pwvmean = defaultPWV
        else:
              pwvmean = defaultPWV
              if (missingCalWVRErrorPrinted == False):
                  missingCalWVRErrorPrinted = True
                  if (telescopeName.find('ALMA')>=0):
                      print "No ASDM_CALWVR or CalWVR.xml table found.  Using PWV %.1fmm." % pwvmean
                  else:
                      print "This telescope has no WVR to provide a PWV measurement. Using PWV %.1fmm." % pwvmean
      else:
          try:
              pwvmean = float(pwv)
          except:
              pwvmean = defaultPWV
    else:
          try:
              pwvmean = float(pwv)
          except:
              pwvmean = defaultPWV

    if (verbose):
        print "Using PWV = %.2f mm" % pwvmean

    # default values in case we can't find them below
    airmass = 1.5
    P = 563.0
    H = 20.0
    T = 273.0


    if (verbose):
          print "Looking for scans for field integer = %d" % (field)
    scans = vm.getScansForFieldID(field)
    if (verbose):
          print "For field %s, Got scans = " % str(field),scans
    scantimes = vm.getTimesForScans(scans)
    mindiff = 1e20
    for i in range(len(scantimes)):
          stime = scantimes[i]
          meantime = np.mean(stime)
          tdiff = np.abs(meantime-timestamp)
          if (tdiff < mindiff):
              bestscan = scans[i]
              mindiff = tdiff
    if (verbose):
          print "For timestamp=%.1f, got closest scan = %d, %.0f sec away" %(timestamp, bestscan,mindiff)
    [conditions,myTimes,vm] = au.getWeather(vis,bestscan,antenna,verbose,vm)
    P = conditions['pressure']
    H = conditions['humidity']
    T = conditions['temperature']+273.15
    if (P <= 0.0):
        P = 563
    if (H <= 0.0):
        H = 20
    if (('elevation' in conditions.keys()) == False):
        # Someone cleared the POINTING table, so calculate elevation from Ra/Dec/MJD
        myfieldId =  vm.getFieldIdsForFieldName(vm.getFieldsForScan(bestscan))
        myscantime = np.mean(vm.getTimesForScans(bestscan))
        mydirection = au.getRADecForField(vis, myfieldId)
        if (verbose):
            print "Scan =  %d, time = %.1f,  Field = %d, direction = " % (bestscan, myscantime, myfieldId), mydirection
        telescopeName = au.getObservatoryName(vis)
        if (len(telescopeName) < 1):
            telescopeName = 'ALMA'
        myazel = au.computeAzElFromRADecMJD(mydirection, myscantime/86400., telescopeName)
        conditions['elevation'] = myazel[1] * 180/math.pi
        conditions['azimuth'] = myazel[0] * 180/math.pi
        if (verbose):
            print "Computed elevation = %.1f deg" % (conditions['elevation'])
        
    if (verbose):
          print "CalcAtm: found elevation=%f (airmass=%.3f) for scan:" % (conditions['elevation'],1/np.sin(conditions['elevation']*np.pi/180.)), bestscan
          print "P,H,T = %f,%f,%f" % (P,H,T)
    if (conditions['elevation'] <= 3):
        print "Using 45 deg elevation instead"
        airmass = 1.0/math.cos(45*math.pi/180.)
    else:
        airmass = 1.0/math.cos((90-conditions['elevation'])*math.pi/180.)

    # Use the correct polarization data if only one is present
    # The missing data will have a zero-length array of channels.
    if (len(pchan[0])>1):
        chans = pchan[0]
        freqs = pfreq[0]
    else:
        chans = pchan[1]
        freqs = pfreq[1]

    tropical = 1
    midLatitudeSummer = 2
    midLatitudeWinter = 3
#    at.initAtmProfile(humidity=H,temperature=casac.Quantity(T,"K"),altitude=casac.Quantity(5059,"m"),pressure=casac.Quantity(P,'mbar'),atmType=midLatitudeWinter)
    numchan=len(freqs)
    reffreq=0.5*(freqs[numchan/2-1]+freqs[numchan/2])
    originalnumchan = numchan
    while (numchan > 512):
        numchan /= 2
#        print "Reduce numchan to ", numchan
        chans = range(0,originalnumchan,(originalnumchan/numchan))
    chansep = (freqs[-1]-freqs[0])/(numchan-1)
    nbands = 1
    fCenter = casac.Quantity(reffreq,'GHz')
    fResolution = casac.Quantity(chansep,'GHz')
    fWidth = casac.Quantity(numchan*chansep,'GHz')
    at.initAtmProfile(humidity=H,temperature=casac.Quantity(T,"K"),altitude=casac.Quantity(5059,"m"),pressure=casac.Quantity(P,'mbar'),atmType=midLatitudeWinter)
    at.initSpectralWindow(nbands,fCenter,fWidth,fResolution)
    at.setUserWH2O(casac.Quantity(pwvmean,'mm'))

#    at.setAirMass()  # This does not affect the opacity, but it does effect TebbSky, so do it manually.

  # readback the values to be sure they got set
    rf = at.getRefFreq()
    rc = at.getRefChan()
    cs = at.getChanSep()
    n = at.getNumChan()
    chans = range(0,n)
#    chans = list(np.array(range(0,n)) + 0.5)
    dry = np.array(at.getDryOpacitySpec(0)['dryOpacity'])
    wet = np.array(at.getWetOpacitySpec(0)['wetOpacity'].value)
    try:
       TebbSky = at.getTebbSkySpec(spwid=0).value
    except:
#       print "Getting Tsky, channel by channel"
       TebbSky = []
       for chan in range(n):  # do NOT use numchan here, use n
           TebbSky.append(at.getTebbSky(nc=chan, spwid=0).value)
       TebbSky = np.array(TebbSky)
    transmission = np.exp(-airmass*(wet+dry))
    TebbSky *= (1-np.exp(-airmass*(wet+dry)))/(1-np.exp(-wet-dry))

    if (refFreqInTable*1e-9>np.mean(freqs)):
        if ((net_sideband % 2) == 0):
            sense = 1
        else:
            sense = 2
    else:
        if ((net_sideband % 2) == 0):
            sense = 2
        else:
            sense = 1

    if (sense == 1):
        # The following looks right for LSB   sense=1
        freq = rf.value + cs.value*0.001*(0.5*n-1-np.array(range(n)))
        if (xaxis.find('chan')>=0):
            trans = np.zeros(len(transmission))
            Tebb = np.zeros(len(TebbSky))
            for i in range(len(transmission)):
                trans[i] = transmission[len(transmission)-1-i]
                Tebb[i] = TebbSky[len(TebbSky)-1-i]
            transmission = trans
            TebbSky = Tebb
    else:
        # Using numchan can cause an inconsistency for small number of channels
#        freq = rf.value+cs.value*0.001*(np.array(range(numchan))-0.5*numchan+1)
        # The following looks right for USB  sense=2
        freq = rf.value+cs.value*0.001*(np.array(range(n))-0.5*n+1)
        
    return(freq, chans, transmission, pwvmean, airmass, TebbSky, missingCalWVRErrorPrinted)

def RescaleTrans(trans, lim, subplotRows, lo1=None, xframe=0):
    # Input: the array of transmission or TebbSky values and current limits
    # Returns: arrays of the rescaled transmission values and the zero point
    #          values in units of the frame, and in amplitude.
    debug = False
    yrange = lim[1]-lim[0]
    if (lo1 == None):
        labelgap = 0.6 # Use this fraction of the margin for the PWV ATM label
    else:
        labelgap = 0.5 # Use this fraction of the margin to separate the top
                       # curve from the upper y-axis
    y2 = lim[1] - labelgap*yrange*TOP_MARGIN/(1.0+TOP_MARGIN)
    y1 = lim[1] - yrange*TOP_MARGIN/(1.0+TOP_MARGIN)
    transmissionRange = np.max(trans)-np.min(trans)
    if (transmissionRange < 0.05):
          # force there to be a minimum range of transmission display 
          # overemphasize tiny ozone lines
          transmissionRange = 0.05

    if (transmissionRange > 1 and transmissionRange < 10):
          # force there to be a minimum range of Tebbsky (10K) to display
          transmissionRange = 10

    # convert transmission to amplitude
    newtrans = y2 - (y2-y1)*(np.max(trans)-trans)/transmissionRange

    # Use edge values
    edgeValueTransmission = trans[-1]
    otherEdgeValueTransmission = trans[0]

    # Now convert the edge channels' transmission values into amplitude
    edgeValueAmplitude = y2 - (y2-y1)*(np.max(trans)-trans[-1])/transmissionRange
    otherEdgeValueAmplitude = y2 - (y2-y1)*(np.max(trans)-trans[0])/transmissionRange

    # Now convert amplitude to frame units, offsetting downward by half
    # the font size
    fontoffset = 0.01*subplotRows
    edgeValueFrame = (edgeValueAmplitude - lim[0])/yrange  - fontoffset
    otherEdgeValueFrame = (otherEdgeValueAmplitude - lim[0])/yrange  - fontoffset

    # scaleFactor is how large the plot is from the bottom x-axis
    # up to the labelgap, in units of the transmissionRange
    scaleFactor = (1+TOP_MARGIN*(1-labelgap)) / (TOP_MARGIN*(1-labelgap))

    # compute the transmission at the bottom of the plot, and label it
    y0transmission = np.max(trans) - transmissionRange*scaleFactor
    y0transmissionFrame = 0
    y0transmissionAmplitude = lim[0]

    if (y0transmission <= 0):
        # If the bottom of the plot is below zero transmission, then label
        # the location of zero transmission instead.
        if (debug):
            print ">>>>>>>> y0transmission original = %f, (y1,y2)=(%f,%f)" % (y0transmission,y1,y2)
        y0transmissionAmplitude = y1-(y2-y1)*(np.min(trans)/transmissionRange)
        y0transmissionFrame = (y0transmissionAmplitude-lim[0]) / (lim[1]-lim[0])
        y0transmission = 0
    if (debug):
          print ">>>>>>>> xframe=%d, scaleFactor = " % (xframe), scaleFactor
          print "edgeValueFrame, other = ", edgeValueFrame, otherEdgeValueFrame
          print "edgeValueTransmission, other = ", edgeValueTransmission, otherEdgeValueTransmission
          print "edgeValueAmplitude, otherEdgeValueAmplitude = ", edgeValueAmplitude, otherEdgeValueAmplitude
          print "y0transmission = %f, y0transmissionFrame = %f" % (y0transmission,y0transmissionFrame)
          print "y0transmissionAmplitude = ", y0transmissionAmplitude
          print "transmissionRange = ", transmissionRange
    return(newtrans, edgeValueFrame, y0transmission, y0transmissionFrame,
           otherEdgeValueFrame, edgeValueTransmission,
           otherEdgeValueTransmission, edgeValueAmplitude,
           otherEdgeValueAmplitude, y0transmissionAmplitude)

def RescaleX(chans, lim, plotrange):
    # This function is now not used. - July 24, 2012
    # If the user specified a plotrange, then rescale to this range,
    # otherwise rescale to the automatically-determined range.
    if (chans[1] > chans[0]):
        chanrange = chans[-1]-chans[0]
    else:
        chanrange = chans[0]-chans[-1]
    if (plotrange[0] != 0 or plotrange[1] != 0):
        xrange = plotrange[1]-plotrange[0]
        return(plotrange[0]+(xrange*np.array(chans))/chanrange)
    else:
        xrange=lim[1]-lim[0]
        return(lim[0]+(xrange*np.array(chans))/chanrange)

def recalcYlimitsFreq(chanrange, ylimits, amp, sideband,plotrange,xchannels):
  # Used by plots with xaxis='freq'
  # xchannels are the actual channel numbers of unflagged data, i.e. displayed points
  # amp is actual data plotted
  ylim_debug = False
  if (chanrange[0]==0 and chanrange[1] == 0 and plotrange[2] == 0 and plotrange[3]==0):
      newmin = np.min(amp)
      newmax = np.max(amp)
      newmin = np.min([ylimits[0],newmin])
      newmax = np.max([ylimits[1],newmax])
      ylimits = [newmin, newmax]
  elif ((abs(chanrange[0]) > 0 or abs(chanrange[1]) > 0)):
      plottedChannels = np.intersect1d(xchannels, range(chanrange[0],chanrange[1]+1))
      if (len(plottedChannels) < 1):
          return(ylimits)
      mylist = np.arange(xchannels.index(plottedChannels[0]), 1+xchannels.index(plottedChannels[-1]))
      if (mylist[-1] >= len(amp)):
          # prevent crash if many channels are flagged
          return(ylimits)
      if (ylim_debug):
          print "Starting with limits = ", ylimits
          print "Examining channels: ", mylist
          print "len(amp): ", len(amp)
          print "Examining values: amp[mylist] = ", amp[mylist]
      newmin = np.min(amp[mylist])
      newmax = np.max(amp[mylist])
      newmin = np.min([ylimits[0],newmin])
      newmax = np.max([ylimits[1],newmax])
      #  The following presents a problem with overlays, as it keeps widening forever
#      newmin -= 0.05*(newmax-newmin)
#      newmax += 0.05*(newmax-newmin)
      ylimits = [newmin, newmax]
      if (ylim_debug):
        print "Returning with limits = ", ylimits
  return ylimits

def recalcYlimits(plotrange, ylimits, amp):
  # Used by plots with xaxis='chan'
  if ((abs(plotrange[0]) > 0 or abs(plotrange[1]) > 0) and (plotrange[2] == 0 and plotrange[3] == 0)):
      x0 = plotrange[0]
      x1 = plotrange[1]
      if (x0 < 0):
          x0 = 0
      if (x1 > len(amp)-1):
          x1 = len(amp)-1
      if (len(amp) > x1 and x0 < x1):
          newmin = np.min(amp[x0:x1])
          newmax = np.max(amp[x0:x1])
          newmin = np.min([ylimits[0],newmin])
          newmax = np.max([ylimits[1],newmax])
          ylimits = [newmin, newmax]
  else:
      ylimits = pb.ylim()  # added on 10/27/2011
#      print "current ylimits = ", ylimits
  return(ylimits)

def SetNewYLimits(newylimits):
#    print "Entered SetNewYLimits with ", newylimits 
    newrange = newylimits[1]-newylimits[0]
    pb.ylim([newylimits[0]-0.0*newrange, newylimits[1]+0.0*newrange])

def SetNewXLimits(newxlimits):
#    print "Entered SetNewXLimits with range = %.3f" % (np.max(newxlimits)-np.min(newxlimits))
    xrange = np.abs(newxlimits[1]-newxlimits[0])
    buffer = 0.01
    if (newxlimits[0] < newxlimits[1]):
        pb.xlim([newxlimits[0]-xrange*buffer,newxlimits[1]+xrange*buffer] )
    else:
#        print "Swapping xlimits order"
        pb.xlim(newxlimits[1]-xrange*buffer, newxlimits[0]+xrange*buffer)

def sloppyMatch(newvalue, mylist, threshold, myprint=False):
    matched = False
    if (type(mylist) != list and type(mylist)!=np.ndarray):
	mylist = [mylist]
    for v in mylist:
        if (abs(newvalue-v) < threshold):
            matched = True
    
    if (matched == False and myprint==True):
        print "sloppyMatch: %.0f is not within %.0f of anything in " % (newvalue,threshold), mylist
#        print "Returning matched = ", matched
    elif (myprint==True):
        print "sloppyMatch: %.0f is within %.0f of something in " % (newvalue,threshold), mylist
#        print "Returning matched = ", matched
    return(matched)

def sloppyUnique(t, thresholdSeconds):
    """
    Takes a list of numbers and returns a list of unique values, subject to a threshold difference.
    """
    # start with the first entry, and only add a new entry if it is more than the threshold from prior
    sloppyList = [t[0]]
    for i in range(1,len(t)):
        keepit = True
        for j in range(0,i):
            if (abs(t[i]-t[j]) < thresholdSeconds):
                keepit = False
        if (keepit):
            sloppyList.append(t[i])
#    print "sloppyUnique returns %d values from the original %d" % (len(sloppyList), len(t))
    return(sloppyList)

def SetLimits(plotrange, chanrange, newylimits, channels, frequencies, pfrequencies, ampMin, ampMax, xaxis, pxl):
    if (abs(plotrange[0]) > 0 or abs(plotrange[1]) > 0):
        SetNewXLimits([plotrange[0],plotrange[1]])
        if (plotrange[2] == 0 and plotrange[3] == 0):
            # reset the ylimits based on the channel range shown (selected via plotrange)
            SetNewYLimits(newylimits)
    else: # set xlimits to full range
        if (xaxis.find('chan')>=0):
            SetNewXLimits([channels[0],channels[-1]])
        else:
#            print "SetLimits(): Setting x limits to full range (%f-%f)" % (frequencies[0], frequencies[-1])
            SetNewXLimits([frequencies[0], frequencies[-1]])
    if (chanrange[0] != 0 or chanrange[1] != 0):
        # reset the ylimits based on the channel range specified (selected via chanrange)
        if (newylimits != [LARGE_POSITIVE, LARGE_NEGATIVE]):
            SetNewYLimits(newylimits)
#        print "pxl=%d, chanrange[0]=%d, chanrange[1]=%d, shape(pfreq), shape(freq)=" % (pxl, chanrange[0], chanrange[1]), np.shape(pfrequencies),np.shape(frequencies)
        # Use frequencies instead of pfrequencies, because frequencies are not flagged and
        # will continue to work if chanranze is specified and data are flagged.
        try:
            SetNewXLimits([frequencies[chanrange[0]], frequencies[chanrange[1]]])  # Apr 3, 2012
#            SetNewXLimits([pfrequencies[pxl][chanrange[0]], pfrequencies[pxl][chanrange[1]]])
        except:
            print "Invalid chanrange (%d-%d). Valid range = 0-%d" % (chanrange[0],chanrange[1],len(frequencies)-1)
            return(-1)
    if (abs(plotrange[2]) > 0 or abs(plotrange[3]) > 0):
        SetNewYLimits([plotrange[2],plotrange[3]])
    return(0)
    
def showFDM(originalSpw, chanFreqGHz):
    """
    Draws a horizontal bar indicating the location of FDM spws in the dataset.
    """

    # add some space at the bottom -- Apr 25, 2012
    ylim = pb.ylim()
    yrange = ylim[1]-ylim[0]
    pb.ylim([ylim[0]-BOTTOM_MARGIN*yrange, ylim[1]])

#    print "Showing FDM (%d)" % (len(originalSpw)), originalSpw
    fdmctr = -1
    x0,x1 = pb.xlim()
    y0,y1 = pb.ylim()
    yrange = y1 - y0
    pb.hold(True)
    for i in range(len(originalSpw)):
        if (len(chanFreqGHz[i]) > 256):
            fdmctr += 1
            verticalOffset = fdmctr*0.04*yrange
            y1a = y0 + 0.03*yrange + verticalOffset
            y2 = y1a + 0.01*yrange
#            print "chan=%d: Drawing line at y=%f (y0=%f) from x=%f to %f" % (len(chanFreqGHz[i]),
#                                              y1a,y0,chanFreqGHz[i][0], chanFreqGHz[i][-1])
            f0 = chanFreqGHz[i][0]
            f1 = chanFreqGHz[i][-1]
            if (f1 < f0):
                swap = f1
                f1 = f0
                f0 = swap
            v0 = np.max([f0,x0])
            v1 = np.min([f1,x1])
            if (v1 > v0):
                xlabel = 0.5*(v0+v1)
                if (xlabel < x0):
                    xlabel = x0
                if (xlabel > x1):
                    xlabel = x1
                pb.plot([v0,v1], [y1a,y1a], '-',
                        linewidth=4, color=overlayColors[fdmctr])
                pb.text(xlabel, y2, "spw%d"%(i), size=7)
    if (fdmctr > -1):
        pb.ylim([y0,y1])
        pb.xlim([x0,x1])

def DrawAtmosphere(showatm, showtsky, subplotRows, atmString, mysize,
                   TebbSky, plotrange, xaxis, atmchan, atmfreq, transmission,
                   subplotCols, lo1=None, xframe=0, firstFrame=0,
                   showatmPoints=False):
    """
    Draws atmospheric transmission or Tsky on an amplitude vs. chan or freq plot.
    """
    xlim = pb.xlim()
    ylim = pb.ylim()
    xrange = xlim[1]-xlim[0]
    yrange = ylim[1]-ylim[0]

    if (lo1 == None):
        # add some space at the top -- Apr 16, 2012
        pb.ylim([ylim[0], ylim[1]+TOP_MARGIN*yrange])
    else:
        pb.ylim([ylim[0], ylim[1]+TOP_MARGIN*yrange*0.5])
    ylim = pb.ylim()
    yrange = ylim[1]-ylim[0]
    #
    ystartPolLabel = 1.0-0.04*subplotRows
    if (lo1 == None):
        transmissionColor = 'm'
        tskyColor = 'm'
    else:
        transmissionColor = 'k'
        tskyColor = 'k'
    if (showatmPoints):
        atmline = '.'
    else:
        atmline = '-'
    if (showatm or showtsky):
        if (showatm):
            atmcolor = transmissionColor
        else:
            atmcolor = tskyColor
        if (lo1 == None):
            pb.text(0.25, ystartPolLabel, atmString, color=atmcolor, size=mysize, transform=pb.gca().transAxes)
        if (showtsky):
            rescaledY, edgeYvalue, zeroValue, zeroYValue, otherEdgeYvalue, edgeT, otherEdgeT, edgeValueAmplitude, otherEdgeValueAmplitude, zeroValueAmplitude = RescaleTrans(TebbSky, ylim, subplotRows, lo1, xframe)
        else:
            rescaledY, edgeYvalue, zeroValue, zeroYValue, otherEdgeYvalue, edgeT, otherEdgeT, edgeValueAmplitude, otherEdgeValueAmplitude, zeroValueAmplitude = RescaleTrans(transmission, ylim, subplotRows, lo1, xframe)
        if (xaxis.find('chan')>=0):
#            rescaledX = RescaleX(atmchan, xlim, plotrange)
            rescaledX = atmchan
            pb.plot(rescaledX, rescaledY,'%s%s'%(atmcolor,atmline))
            tindex = -1
        elif (xaxis.find('freq')>=0):
            pb.plot(atmfreq, rescaledY, '%s%s'%(atmcolor,atmline))
            if (atmfreq[0]<atmfreq[1]):
                tindex = -1
            else:
                tindex = 0
        if (lo1 == None):
              xEdgeLabel = 1.01
        else:
            if (xframe == firstFrame):
                xEdgeLabel = -0.10*subplotCols # avoids overwriting y-axis label
            else:
                xEdgeLabel = -0.10*subplotCols
        SetNewXLimits(xlim)  # necessary for zoom='intersect'
        SetNewYLimits(ylim)
        # Now draw the percentage on right edge of plot
        if (showtsky):
            if (lo1 == None):
#                pb.text(xEdgeLabel, edgeYvalue,'%.0fK'%(edgeT),color=atmcolor,
#                        size=mysize, transform=pb.gca().transAxes)
#                pb.text(1.01, zeroYValue,'%.0fK'%(zeroValue), color=atmcolor,
#                        size=mysize, transform=pb.gca().transAxes)
                # This must be done in user coordinates since another curve
                # is plotted following this one.
                pb.text(xlim[1]+0.06*xrange/subplotCols, edgeValueAmplitude,
                        '%.0fK'%(edgeT), color=atmcolor, size=mysize)
                pb.text(xlim[1]+0.06*xrange/subplotCols, zeroValueAmplitude,
                        '%.0fK'%(zeroValue), color=atmcolor,
                        size=mysize)
            else:
                # This can remain in axes units since it is the final plot.
                pb.text(xEdgeLabel, otherEdgeYvalue,'%.0fK'%(otherEdgeT),
                        color=atmcolor,
                        size=mysize, transform=pb.gca().transAxes)
                pb.text(xEdgeLabel, zeroYValue,'%.0fK'%(zeroValue),
                        color=atmcolor,
                        size=mysize, transform=pb.gca().transAxes)
        else:
            # showatm=True
            if (lo1 == None):
#                pb.text(xEdgeLabel, edgeYvalue,'%.0f%%'%(edgeT*100),
#                        color=atmcolor,
#                        size=mysize, transform=pb.gca().transAxes)
                # This must be done in user coordinates since another curve
                # is plotted following this one.
                pb.text(xlim[1]+0.05*xrange/subplotCols, edgeValueAmplitude,
                        '%.0f%%'%(edgeT*100), color=atmcolor, size=mysize)
                pb.text(xlim[1]+0.05*xrange/subplotCols, zeroValueAmplitude,
                        '%.0f%%'%(zeroValue*100), color=atmcolor,
                        size=mysize)
            else:
                # This can remain in axes units since it is the final plot.
                pb.text(xEdgeLabel, otherEdgeYvalue,'%.0f%%'%(otherEdgeT*100),
                        color=atmcolor,
                        size=mysize, transform=pb.gca().transAxes)
                pb.text(xEdgeLabel, zeroYValue,'%.0f%%'%(zeroValue*100),
                        color=atmcolor,
                        size=mysize, transform=pb.gca().transAxes)
        if (lo1 != None):
            if (xframe == firstFrame):
                pb.text(+0.96-0.08*subplotCols, -0.07*subplotRows,
                        'Signal Sideband', color='m', size=mysize,
                        transform=pb.gca().transAxes)
                pb.text(-0.08*subplotCols, -0.07*subplotRows,
                        'Image Sideband', color='k', size=mysize,
                        transform=pb.gca().transAxes)
                

def DrawBottomLegendPageCoords(msName, uniqueTimesMytime, mysize):
    msName = msName.split('/')[-1]
    bottomLegend = msName + '  ObsDate=' + utdatestring(uniqueTimesMytime) + '   plotbandpass2 v' \
                  + PLOTBANDPASS_REVISION_STRING.split()[2] + ' = ' \
                  + PLOTBANDPASS_REVISION_STRING.split()[3] + ' ' \
                  + PLOTBANDPASS_REVISION_STRING.split()[4]
    pb.text(0.1, 0.02, bottomLegend, size=mysize, transform=pb.gcf().transFigure)

def DrawAntennaNames(msAnt, antennasToPlot, msFound, mysize):
    for a in range(len(antennasToPlot)):
        if (msFound):
            legendString = msAnt[antennasToPlot[a]]
        else:
            legendString = str(antennasToPlot[a])
        if (a<maxAntennaNamesAcrossTheTop):
            x0 = xstartTitle+(a*antennaHorizontalSpacing)
            y0 = ystartOverlayLegend
        else:
            # start going down the righthand side
            x0 = xstartTitle+(maxAntennaNamesAcrossTheTop*antennaHorizontalSpacing)
            y0 = ystartOverlayLegend-(a-maxAntennaNamesAcrossTheTop)*antennaVerticalSpacing
        pb.text(x0, y0, legendString,color=overlayColors[a],fontsize=mysize,
                transform=pb.gcf().transFigure)
    
def fixGapInSpws(spws, verbose=False):
                    # find gap in spectralWindowId numbering in ASDM_RECEIVER
                    gap = 0
                    for i in range(len(spws)):
                        if (spws[i] > 0):
                            gap = spws[i] - 1
                            break
                    for i in range(len(spws)):
                        if (spws[i] > 0):
                            if (verbose):
                                print "Renaming spw%d to spw%d" % (spws[i],spws[i]-gap)
                            spws[i] -= gap
                    return(spws)

