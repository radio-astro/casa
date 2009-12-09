import os
import re
import sys
import casac

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
__quantahome__ = casac.homefinder.find_home_by_name('quantaHome')
__measureshome__ = casac.homefinder.find_home_by_name('measuresHome')
__imagerhome__ = casac.homefinder.find_home_by_name('imagerHome')
__calibraterhome__ = casac.homefinder.find_home_by_name('calibraterHome')
__mshome__ = casac.homefinder.find_home_by_name('msHome')
__tableplothome__ = casac.homefinder.find_home_by_name('tableplotHome')
__msplothome__ = casac.homefinder.find_home_by_name('msplotHome')
__plotmshome__ = casac.homefinder.find_home_by_name('plotmsHome')
__calplothome__ = casac.homefinder.find_home_by_name('calplotHome')
__tablehome__ = casac.homefinder.find_home_by_name('tableHome')
__flaggerhome__ = casac.homefinder.find_home_by_name('flaggerHome')
__autoflaghome__ = casac.homefinder.find_home_by_name('autoflagHome')
__imagehome__ = casac.homefinder.find_home_by_name('imageHome')
__imagepolhome__ = casac.homefinder.find_home_by_name('imagepolHome')
__simulatorhome__ = casac.homefinder.find_home_by_name('simulatorHome')
__componentlisthome__ = casac.homefinder.find_home_by_name('componentlistHome')
__coordsyshome__ = casac.homefinder.find_home_by_name('coordsysHome')
__regionmanagerhome__ = casac.homefinder.find_home_by_name('regionmanagerHome')
__utilshome__ = casac.homefinder.find_home_by_name('utilsHome')
__deconvolverhome__ = casac.homefinder.find_home_by_name('deconvolverHome')
__vpmanagerhome__ = casac.homefinder.find_home_by_name('vpmanagerHome')
__vlafillertaskhome__ = casac.homefinder.find_home_by_name('vlafillertaskHome')
__atmospherehome__ = casac.homefinder.find_home_by_name('atmosphereHome')

quanta = __quantahome__.create( )
measures = __measureshome__.create( )
imager = __imagerhome__.create( )
calibrater = __calibraterhome__.create( )
ms = __mshome__.create( )
tableplot = __tableplothome__.create( )
msplot = __msplothome__.create( )
pmtool = __plotmshome__.create( )
calplot = __calplothome__.create( )
table = __tablehome__.create( )
flagger = __flaggerhome__.create( )
autoflag = __autoflaghome__.create( )
image = __imagehome__.create( )
imagepol = __imagepolhome__.create( )
simulator = __simulatorhome__.create( )
componentlist = __componentlisthome__.create( )
coordsys = __coordsyshome__.create( )
regionmanager = __regionmanagerhome__.create( )
utils = __utilshome__.create( )
deconvolver = __deconvolverhome__.create( )
vpmanager = __vpmanagerhome__.create( )
vlafillertask = __vlafillertaskhome__.create( )
atmosphere = __atmospherehome__.create( )

from accum import accum
from applycal import applycal
from bandpass import bandpass
from blcal import blcal
from browsetable import browsetable
from calstat import calstat
from clean import clean
from clearcal import clearcal
from clearplot import clearplot
from clearstat import clearstat
from concat import concat
from deconvolve import deconvolve
from exportfits import exportfits
from exportuvfits import exportuvfits
from feather import feather
from find import find
from flagautocorr import flagautocorr
from flagdata import flagdata
from flagmanager import flagmanager
from fringecal import fringecal
from ft import ft
from fluxscale import fluxscale
from gaincal import gaincal
from hanningsmooth import hanningsmooth
from imcontsub import imcontsub
from imfit import imfit
from imhead import imhead
from immath import immath
from immoments import immoments
from importasdm import importasdm
from importfits import importfits
from importgmrt import importgmrt
from importvla import importvla
from importuvfits import importuvfits
from imregrid import imregrid
from imstat import imstat
from imval import imval
from listcal import listcal
from listhistory import listhistory
from listobs import listobs
from listvis import listvis
from makemask import makemask
from mosaic import mosaic
from newclean import newclean
from newflagdata import newflagdata
from oldclean import oldclean
from plotants import plotants
from plotcal import plotcal
from plotxy import plotxy
from polcal import polcal
from plotms import plotms
from setjy import setjy
from simdata import simdata
from smoothcal import smoothcal
from specfit import specfit
from split import split
from uvcontsub import uvcontsub
from uvmodelfit import uvmodelfit
from uvsub import uvsub
from viewer import viewer
from vishead import vishead
from visstat import visstat
from widefield import widefield


##
## asap may not be available with every casa installation
##
try:
    from sdcoadd import sdcoadd
    from sdscale import sdscale
    from sdtpimaging import sdtpimaging
    from sdlist import sdlist
    from sdfit import sdfit
    from sdbaseline import sdbaseline
    from sdstat import sdstat
    from sdsave import sdsave
    from sdflag import sdflag
    from sdaverage import sdaverage
    from sdplot import sdplot
    from sdsmooth import sdsmooth
    from sdcal import sdcal
except ImportError:
    sdcoadd = None
    sdscale = None
    sdtpimaging = None
    sdlist = None
    sdfit = None
    sdbaseline = None
    sdstat = None
    sdsave = None
    sdflag = None
    sdaverage = None
    sdplot = None
    sdsmooth = None
    sdcal = None
