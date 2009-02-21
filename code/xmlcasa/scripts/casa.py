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

from task_imstat import imstat
from task_fringecal import fringecal
from task_oldclean import oldclean
from task_sdcoadd import sdcoadd
from task_flagdata import flagdata
from task_split import split
from task_sdscale import sdscale
from task_importvla import importvla
from task_hanningsmooth import hanningsmooth
from task_widefield import widefield
from task_immoments import immoments
from task_uvcontsub import uvcontsub
from task_listhistory import listhistory
from task_mosaic import mosaic
from task_sdtpimaging import sdtpimaging
from task_importfits import importfits
from task_imfit import imfit
from task_invert import invert
from task_sdlist import sdlist
from task_deconvolve import deconvolve
from task_almasimmos import almasimmos
from task_importasdm import importasdm
from task_uvmodelfit import uvmodelfit
from task_polcal import polcal
from task_clearcal import clearcal
from task_bandpass import bandpass
from task_clearplot import clearplot
from task_sdfit import sdfit
from task_listvis import listvis
from task_sdbaseline import sdbaseline
from task_accum import accum
from task_concat import concat
from task_listobs import listobs
from task_feather import feather
from task_imregrid import imregrid
from task_sdstat import sdstat
from task_newclean import newclean
from task_sdsave import sdsave
from task_listcal import listcal
from task_plotxy import plotxy
from task_ft import ft
from task_importuvfits import importuvfits
from task_sdflag import sdflag
from task_sdaverage import sdaverage
from task_setjy import setjy
from task_clean import clean
from task_sdplot import sdplot
from task_immath import immath
from task_imhead import imhead
from task_sdsmooth import sdsmooth
from task_find import find
from task_gaincal import gaincal
from task_fluxscale import fluxscale
from task_applycal import applycal
from task_plotants import plotants
from task_exportuvfits import exportuvfits
from task_plotcal import plotcal
from task_flagmanager import flagmanager
from task_specfit import specfit
from task_viewer import viewer
from task_exportfits import exportfits
from task_blcal import blcal
from task_uvsub import uvsub
from task_imcontsub import imcontsub
from task_imstat import imstat
from task_newflagdata import newflagdata
from task_clearstat import clearstat
from task_flagautocorr import flagautocorr
from task_browsetable import browsetable
from task_makemask import makemask
from task_smoothcal import smoothcal
from task_imval import imval
from task_sdcal import sdcal
from task_vishead import vishead

