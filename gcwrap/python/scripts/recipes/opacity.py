# opacal()
#
# Recipe to create a frequency dependent opacity calibration table.
# The gencal generated 'opac' table is constant in time and frequency
# (single value per spectral window), this function uses the WEATHER table
# to create an opacity table that can be interpolated in time and frequency.
# This recipe matches the Miriad calculation for the ATCA when the
# height parameter is left at the default and the asap parameter is set
# to True. It may give a reasonable approximation for other observatories
# if the height parameters is adjusted, but some of the atmospheric
# parameters in the code may need changing too.
#
# To access this function, type (at the CASA prompt):
#
# from recipes.opacity import opacal
# or 
# execfile(casadef.python_library_directory+'/recipes/opacity.py')
#
# MHW, Feb 2016
from taskinit import tbtool,cu
from task_gencal import gencal as mygencal
import pylab as pl

def opacal(vis,calname,asap=True,interpolate=0,height=200):
  """Create an opacity calibration table for the MeasurementSet given.
  If calname is empty, one is created based on vis and returned.
  Use asap=True to use the ASAP/Miriad atmosphere model and
  asap=False to use the CASA ATM model.
  Use interpolate>1 to use piecewise linear interpolation across each spectrum
  or leave at 0 to use a constant value per spectral window.
  The height parameter should specify the observatory height above mean
  sea level (not height above WGS84 ellipsoid)"""

  mytb = tbtool()
  # get data we need from weather table
  try:
    mytb.open(vis+'/WEATHER')
    time = mytb.getcol('TIME')
    press = mytb.getcol('PRESSURE') #Pa
    temp = mytb.getcol('TEMPERATURE') # C (sometimes returns Kelvin)
    hum = mytb.getcol('REL_HUMIDITY') # %
  except Exception, e:
    print "Error: Cannot read required data from WEATHER table"
    return
  mytb.close()
  if (time.size==0):
    print "Error: no data in WEATHER table, cannot calculate opacities"
    return
    
  # check array name
  mytb.open(vis+'/OBSERVATION')
  tel=mytb.getcell('TELESCOPE_NAME',0)
  mytb.close()
  # ATCA weather station records pressure adjusted to sea level, ATM code wants
  # actual surface pressure - we undo the (fixed) correction here. ASAP code
  # does the correction itself when height>0.
  pfac=1.0
  if (tel=='ATCA'):
    pfac=0.975
  
  # create opac table and read the frequencies, extend opac table to fit an
  # entry for each timestamp
  if (calname==""):
    calname=vis.split(".")[0]+".opac.caltable"
  cu.removetable(calname)
  mygencal(vis,calname,'opac',infile='',spw='',antenna='',pol='',parameter=[0.0])
  mytb.open(calname+'/SPECTRAL_WINDOW')
  fref=mytb.getcol('REF_FREQUENCY')/1e9
  bw=mytb.getcol('TOTAL_BANDWIDTH')/1e9
  nchan=mytb.getcol('NUM_CHAN')
  mytb.close()
  nf=1
  if (interpolate): 
    nf=max(2,interpolate)
  f=pl.zeros(nf*fref.size)
  print "Using %i frequencies to calculate opacity for each spectral window" % nf
  if (interpolate):
    for i in range(nf):
      f[i::nf]=fref-bw/2+i*bw/(nf-1)
  else:
    f=fref
  mytb.open(calname,nomodify=False)
  spw=mytb.getcol('SPECTRAL_WINDOW_ID')
  ant1=mytb.getcol('ANTENNA1')
  nr=mytb.nrows()
  addnr=nr*(time.size-1)
  mytb.addrows(addnr)
  nrow2=mytb.nrows()

  # work out opacity for each time stamp in weather table and
  # write values to opacity table
  op=pl.zeros(nr*nf)
  print "Opacity table will have %i times with %i ants*spws each" % (time.size,nr)
  for i in range(0,time.size):
    mytb.putcol('TIME',time[i]*pl.ones(nr,float),i*nr,nr)
    mytb.putcol('SPECTRAL_WINDOW_ID',spw,i*nr,nr)
    mytb.putcol('ANTENNA1',ant1,i*nr,nr)
    #import pdb
    #pdb.set_trace()
    tempK = temp[i]
    if tempK<100: tempK+=273.15
    if (asap):
      (fout,opac) = asapatm(f,tempK,hum[i],press[i]/100,height)
    else:
      (fout,opac) = atm(f,tempK,hum[i],pfac*press[i]/100,height)
    for j in range(nr):
      if (interpolate):
        op[j*nf:j*nf+nf]=opac[nf*spw[j]:nf*spw[j]+nf]
      else:
        op[j]=opac[spw[j]]
    # Note: reshape with fortran order to get the right values in each row
    mytb.putcol('FPARAM',op.reshape([1,nf,nr],order='F'),i*nr,nr)
    mytb.putcol('PARAMERR',pl.zeros([1,nf,nr]),i*nr,nr)
    mytb.putcol('FLAG',pl.zeros([1,nf,nr],bool),i*nr,nr)
    mytb.putcol('SNR',pl.ones([1,nf,nr],float),i*nr,nr)
    # fill other columns with default values (0 or -1)
    mytb.putcol('FIELD_ID',-pl.ones(nr,int),i*nr,nr)
    mytb.putcol('ANTENNA2',-pl.ones(nr,int),i*nr,nr)
    mytb.putcol('OBSERVATION_ID',pl.zeros(nr,int),i*nr,nr)
    mytb.putcol('FIELD_ID',-pl.ones(nr,int),i*nr,nr)
  if (nf>1):
    # Fix VisCal keyword and Table info
    mytb.putkeyword('VisCal','TfOpac')
    info = mytb.info()
    info['subType']='TfOpac'
    mytb.putinfo(info)
  mytb.flush()
  mytb.close()
    
  if (nf>1):
    # Fix spectral window table
    mytb.open(calname+'/SPECTRAL_WINDOW',nomodify=False)
    
    mytb.putcol('CHAN_FREQ',(1e9*fout).reshape([nf,-1],order='F'))
    cw=pl.zeros(nf*bw.size)
    for i in range(nf):
      cw[i::nf]=bw*1e9/nf
    cw=cw.reshape([nf,-1],order='F')
    mytb.putcol('CHAN_WIDTH',cw)
    mytb.putcol('EFFECTIVE_BW',cw)
    mytb.putcol('RESOLUTION',cw)
    nc=nf*pl.ones(mytb.nrows())
    mytb.putcol('NUM_CHAN',nc)
    mytb.close()
  return calname
  

def asapatm(fGHz,tempK,humi,press,height):
  """Use the asap (=miriad) atmosphere model to calculate the opacity
  given the surface weather data and frequency. fGHz needs to be an array"""
  from asap import opacity
  atm=opacity.model()
  atm.set_observatory_elevation(height)
  atm.set_weather(tempK,press,humi/100.)
  fHz=[f*1.e9 for f in fGHz]
  op=atm.get_opacities(fHz)
  op=pl.array(op)
  return(fGHz,op)  
  
# Fill out Narrabri/ATCA parameters similar to those used in Miriad opacGet
def atm(freqGHz,temp,humi,press,height):
  """Use the ATM model in CASA to calculate the opacity given the
  surface weather data and frequency. Temperature should be in Kelvin,
  pressure should be actual surface pressure in mbar (not adjusted to equivalent
  sea level pressure) and height should be in meters above mean sea level"""
  tmp = qa.quantity(temp, 'K')
  pre = qa.quantity(press, 'mbar')
  hum = humi
  alt = qa.quantity(height, 'm')
  h0  = qa.quantity(1.54, 'km')
  wvl = qa.quantity(-6.5, 'K/km')
  mxA = qa.quantity(10, 'km')
  dpr = qa.quantity(10.0, 'mbar')
  dpm = 1.2
  att = 3 # 3 = mid lat, winter
  myatm = at.initAtmProfile(alt, tmp, pre, mxA, hum, wvl, dpr, dpm, h0, att)

  # set spectral range to compute values for
  nb = len(freqGHz)
  fC = qa.quantity(freqGHz, 'GHz')
  fW = qa.quantity(pl.ones(nb), 'GHz')
  fR = qa.quantity(pl.zeros(nb), 'GHz')
  at.initSpectralWindow(nb, fC, fW, fR)
  fr=pl.zeros(nb)
  op=pl.zeros(nb)
  for i in range(nb):
    fr[i] = at.getSpectralWindow(i)['value']/1e9
    op[i] = at.getDryOpacitySpec(i)[1]+at.getWetOpacitySpec()[1]['value']
  return (fr,op)
