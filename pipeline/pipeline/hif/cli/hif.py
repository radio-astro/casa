#
# User defined tasks setup.
# Generated from buildmytask.
#

import sys
from casa_stack_manip import stack_frame_find

if sys.path[1] != '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli':
  sys.path.insert(1, '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli')
from odict import odict
if not globals().has_key('mytasks') :
  mytasks = odict()

mytasks['hif_antpos'] = 'Derive an antenna position calibration table'
mytasks['hif_applycal'] = 'Apply the calibration(s) to the data'
mytasks['hif_atmflag'] = 'Flag channels with bad atmospheric transmission'
mytasks['hif_bandpass'] = 'Compute bandpass calibration solutions'
mytasks['hif_checkproductsize'] = 'Check imaging product size'
mytasks['hif_correctedampflag'] = 'Flag corrected - model amplitudes based on calibrators.'
mytasks['hif_editimlist'] = 'Add to a list of images to be produced with hif_makeimages()'
mytasks['hif_findcont'] = 'Find continuum frequency ranges'
mytasks['hif_gaincal'] = 'Determine temporal gains from calibrator observations'
mytasks['hif_gainflag'] = 'Flag antennas with deviant gain'
mytasks['hif_linfeedpolcal'] = 'Base linfeedpolcal task'
mytasks['hif_lowgainflag'] = 'Flag antennas with low or high gain'
mytasks['hif_makecutoutimages'] = 'Base makecutoutimages task'
mytasks['hif_makeimages'] = 'Compute clean map'
mytasks['hif_makeimlist'] = 'Compute list of clean images to be produced'
mytasks['hif_makermsimages'] = 'Base makermsimages task'
mytasks['hif_mstransform'] = 'Select data from calibrated MS(s) to form new MS(s) for imaging'
mytasks['hif_polarization'] = 'Base polarization task'
mytasks['hif_rawflagchans'] = 'Flag deviant baseline/channels in raw data'
mytasks['hif_refant'] = 'Select the best reference antennas'
mytasks['hif_setjy'] = 'Fill the model column with calibrated visibilities'
mytasks['hif_setmodels'] = 'Set calibrator source models'
mytasks['hif_tclean'] = 'Compute clean map'
mytasks['hif_transformimagedata'] = 'Base transformimagedata task'
mytasks['hif_uvcontfit'] = 'Fit the continuum in the UV plane'
mytasks['hif_uvcontsub'] = 'Subtract the fitted continuum from the data'
mytasks['hpc_hif_applycal'] = 'Apply the calibration(s) to the data'
mytasks['hpc_hif_refant'] = 'Select the best reference antennas'
mytasks['uvcontfit'] = 'Fit the continuum in the UV plane'

if not globals().has_key('task_location') :
  task_location = odict()

task_location['hif_antpos'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_applycal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_atmflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_bandpass'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_checkproductsize'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_correctedampflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_editimlist'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_findcont'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_gaincal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_gainflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_linfeedpolcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_lowgainflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_makecutoutimages'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_makeimages'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_makeimlist'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_makermsimages'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_mstransform'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_polarization'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_rawflagchans'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_refant'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_setjy'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_setmodels'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_tclean'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_transformimagedata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_uvcontfit'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hif_uvcontsub'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hpc_hif_applycal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['hpc_hif_refant'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
task_location['uvcontfit'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli'
myglobals = stack_frame_find( )
tasksum = myglobals['tasksum'] 
for key in mytasks.keys() :
  tasksum[key] = mytasks[key]

from hif_antpos_cli import  hif_antpos_cli as hif_antpos
from hif_applycal_cli import  hif_applycal_cli as hif_applycal
from hif_atmflag_cli import  hif_atmflag_cli as hif_atmflag
from hif_bandpass_cli import  hif_bandpass_cli as hif_bandpass
from hif_checkproductsize_cli import  hif_checkproductsize_cli as hif_checkproductsize
from hif_correctedampflag_cli import  hif_correctedampflag_cli as hif_correctedampflag
from hif_editimlist_cli import  hif_editimlist_cli as hif_editimlist
from hif_findcont_cli import  hif_findcont_cli as hif_findcont
from hif_gaincal_cli import  hif_gaincal_cli as hif_gaincal
from hif_gainflag_cli import  hif_gainflag_cli as hif_gainflag
from hif_linfeedpolcal_cli import  hif_linfeedpolcal_cli as hif_linfeedpolcal
from hif_lowgainflag_cli import  hif_lowgainflag_cli as hif_lowgainflag
from hif_makecutoutimages_cli import  hif_makecutoutimages_cli as hif_makecutoutimages
from hif_makeimages_cli import  hif_makeimages_cli as hif_makeimages
from hif_makeimlist_cli import  hif_makeimlist_cli as hif_makeimlist
from hif_makermsimages_cli import  hif_makermsimages_cli as hif_makermsimages
from hif_mstransform_cli import  hif_mstransform_cli as hif_mstransform
from hif_polarization_cli import  hif_polarization_cli as hif_polarization
from hif_rawflagchans_cli import  hif_rawflagchans_cli as hif_rawflagchans
from hif_refant_cli import  hif_refant_cli as hif_refant
from hif_setjy_cli import  hif_setjy_cli as hif_setjy
from hif_setmodels_cli import  hif_setmodels_cli as hif_setmodels
from hif_tclean_cli import  hif_tclean_cli as hif_tclean
from hif_transformimagedata_cli import  hif_transformimagedata_cli as hif_transformimagedata
from hif_uvcontfit_cli import  hif_uvcontfit_cli as hif_uvcontfit
from hif_uvcontsub_cli import  hif_uvcontsub_cli as hif_uvcontsub
from hpc_hif_applycal_cli import  hpc_hif_applycal_cli as hpc_hif_applycal
from hpc_hif_refant_cli import  hpc_hif_refant_cli as hpc_hif_refant
from uvcontfit_cli import  uvcontfit_cli as uvcontfit
