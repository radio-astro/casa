import os
import re
import sys
from casac import *

##
## first set up CASAPATH
##
if os.environ.has_key('CASAPATH') :
    __casapath__ = os.environ['CASAPATH'].split(' ')[0]
    if not os.path.exists(__casapath__ + "/data") :
        raise RuntimeError, "CASAPATH environment variable is improperly set"
else :
    __casapath__ = casac.__file__
    while __casapath__ and __casapath__ != "/" :
        if os.path.exists( __casapath__ + "/data") :
            break
        __casapath__ = os.path.dirname(__casapath__)
    if __casapath__ and __casapath__ != "/" :
        os.environ['CASAPATH']=__casapath__ + " linux local host"
    else :
        raise RuntimeError, "CASAPATH environment variable must be set"

##
## next adjust the PYTHONPATH
##
if re.match( r'.*/\d+\.\d+\.\d+\w*-\d+$', __casapath__ ) :
    for root, dirs, files in os.walk(os.path.dirname(__casapath__)):
        if root.endswith("/numpy"):
            sys.path.append(os.path.dirname(root))
            break
else:
    for root, dirs, files in os.walk(__casapath__):
        if root.endswith("/numpy"):
            sys.path.append(os.path.dirname(root))
            break

##
## next adjust PATH and LD_LIBRARY_PATH
##
for root, dirs, files in os.walk(__casapath__):
    if root.endswith("/bin") and "casapyinfo" in files :
        __ipcontroller__ = (lambda fd: fd.readline().strip('\n'))(os.popen(root + "/casapyinfo --exec 'which ipcontroller'"))
        if os.path.exists(__ipcontroller__) :
            os.environ['PATH'] = os.path.dirname(__ipcontroller__) + ":" + os.environ['PATH']
        else :
            raise RuntimeError, "cannot configure CASA tasking system"
        __ld_library_path__ = (lambda fd: fd.readline().strip('\n').split(':'))(os.popen(root + "/casapyinfo --exec 'echo $LD_LIBRARY_PATH'"))
        map(lambda x: sys.path.append(x),__ld_library_path__)
        break

##
## finally load tools
##

quanta = casac.quanta()
measures = casac.measures()
imager = casac.imager()
calibrater = casac.calibrater()
ms = casac.ms()
tableplot = casac.tableplot()
msplot = casac.msplot()
pmtool = casac.plotms()
calplot = casac.calplot()
table = casac.table()
#flagger = casac.flagger()
agentflagger = casac.agentflagger()
image = casac.image()
imagepol = casac.imagepol()
simulator = casac.simulator()
componentlist = casac.componentlist()
coordsys = casac.coordsys()
regionmanager = casac.regionmanager()
spectralline = casac.spectralline()
utils = casac.utils()
deconvolver = casac.deconvolver()
vpmanager = casac.vpmanager()
vlafillertask = casac.vlafillertask()
atmosphere = casac.atmosphere()
mstransformer = casac.mstransformer()

from accum import  accum
from applycal import  applycal
from asdmsummary import  asdmsummary
from autoclean import  autoclean
from bandpass import  bandpass
from blcal import  blcal
from boxit import  boxit
from browsetable import  browsetable
from calstat import  calstat
from clean import  clean
from clearcal import  clearcal
from clearplot import  clearplot
from clearstat import  clearstat
#from concat import  concat
from cvel import  cvel
from deconvolve import  deconvolve
from exportfits import  exportfits
from exportuvfits import  exportuvfits
from feather import  feather
from find import  find
from fixvis import  fixvis
from flagdata import  flagdata
from flagmanager import  flagmanager
from fluxscale import  fluxscale
from ft import  ft
from gaincal import  gaincal
from gencal import  gencal
from hanningsmooth import  hanningsmooth
from hanningsmooth2 import  hanningsmooth2
from imcontsub import  imcontsub
from imfit import  imfit
from imhead import  imhead
from immath import  immath
from immoments import  immoments
from importasdm import  importasdm
from importfits import  importfits
from importgmrt import  importgmrt
from importuvfits import  importuvfits
from importvla import  importvla
from imregrid import  imregrid
from imsmooth import  imsmooth
from imstat import  imstat
from imval import  imval
from imview import imview
from listcal import  listcal
from listhistory import  listhistory
from listobs import  listobs
from listvis import  listvis
from mosaic import  mosaic
from msmoments import msmoments
from mstransform import mstransform
from msview import msview
from partition import partition
from plotants import  plotants
from plotcal import  plotcal
from plotms import  plotms
#from plotxy import  plotxy
from polcal import  polcal
from rmtables import  rmtables
from setjy import  setjy
from simalma import  simalma
from simobserve import  simobserve
from simanalyze import  simanalyze
from smoothcal import  smoothcal
from specfit import  specfit
from split import  split
from uvcontsub import  uvcontsub
from uvmodelfit import  uvmodelfit
from uvsub import  uvsub
from viewer import  viewer
from vishead import  vishead
from visstat import  visstat
from widefield import  widefield


##
## asap may not be available with every casa installation
##
try:
    import asap as sd
    from sdcal import sdcal
    from sdsmooth import sdsmooth
    from sdbaseline import sdbaseline
    from sdreduce import sdreduce
    from sdcoadd import sdcoadd
    from sdsave import sdsave
    from sdscale import sdscale
    from sdfit import sdfit
    from sdplot import sdplot
    from sdstat import sdstat
    from sdlist import sdlist
    from sdflag import sdflag
    from sdflag2 import sdflag2
    from sdtpimaging import sdtpimaging
    from sdmath import sdmath
    from sdimaging import sdimaging
    from sdimprocess import sdimprocess
except ImportError:
    sd = None
    sdcoadd = None
    sdscale = None
    sdtpimaging = None
    sdlist = None
    sdfit = None
    sdbaseline = None
    sdstat = None
    sdsave = None
    sdflag = None
    sdflag2 = None
    sdcal = None
    sdplot = None
    sdsmooth = None
    sdreduce = None
