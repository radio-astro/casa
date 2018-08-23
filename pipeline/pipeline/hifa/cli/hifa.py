#
# User defined tasks setup.
# Generated from buildmytask.
#

import sys
from casa_stack_manip import stack_frame_find

if sys.path[1] != '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli':
  sys.path.insert(1, '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli')
from odict import odict
if not globals().has_key('mytasks') :
  mytasks = odict()

mytasks['hifa_antpos'] = 'Derive an antenna position calibration table'
mytasks['hifa_bandpass'] = 'Compute bandpass calibration solutions'
mytasks['hifa_bandpassflag'] = 'Compute bandpass calibration with flagging'
mytasks['hifa_bpsolint'] = 'Compute optimal bandpass calibration solution intervals'
mytasks['hifa_exportdata'] = 'Prepare interferometry data for export'
mytasks['hifa_flagdata'] = 'Do basic flagging'
mytasks['hifa_flagtargets'] = 'Do science target flagging'
mytasks['hifa_fluxcalflag'] = 'Locate and flag line regions in solar system flux calibrators'
mytasks['hifa_fluxdb'] = 'Connect to flux calibrator database'
mytasks['hifa_gaincalsnr'] = 'Compute gaincal signal to noise ratios per spw'
mytasks['hifa_gfluxscale'] = 'Derive flux density scales from standard calibrators'
mytasks['hifa_gfluxscaleflag'] = 'Derive the flux density scale with flagging'
mytasks['hifa_imageprecheck'] = 'Base imageprecheck task'
mytasks['hifa_importdata'] = 'Imports data into the interferometry pipeline'
mytasks['hifa_linpolcal'] = 'Compute polarization calibration'
mytasks['hifa_restoredata'] = 'Restore flagged and calibration interferometry data from a pipeline run'
mytasks['hifa_session_bandpass'] = 'Compute bandpass calibration solutions'
mytasks['hifa_spwphaseup'] = 'Compute phase calibration spw map and per spw phase offsets'
mytasks['hifa_timegaincal'] = 'Determine temporal gains from calibrator observations'
mytasks['hifa_tsysflag'] = 'Flag deviant system temperature measurements'
mytasks['hifa_wvrgcal'] = ''
mytasks['hifa_wvrgcalflag'] = ''

if not globals().has_key('task_location') :
  task_location = odict()

task_location['hifa_antpos'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_bandpass'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_bandpassflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_bpsolint'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_exportdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_flagdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_flagtargets'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_fluxcalflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_fluxdb'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_gaincalsnr'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_gfluxscale'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_gfluxscaleflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_imageprecheck'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_importdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_linpolcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_restoredata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_session_bandpass'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_spwphaseup'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_timegaincal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_tsysflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_wvrgcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
task_location['hifa_wvrgcalflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifa/cli'
myglobals = stack_frame_find( )
tasksum = myglobals['tasksum'] 
for key in mytasks.keys() :
  tasksum[key] = mytasks[key]

from hifa_antpos_cli import  hifa_antpos_cli as hifa_antpos
from hifa_bandpass_cli import  hifa_bandpass_cli as hifa_bandpass
from hifa_bandpassflag_cli import  hifa_bandpassflag_cli as hifa_bandpassflag
from hifa_bpsolint_cli import  hifa_bpsolint_cli as hifa_bpsolint
from hifa_exportdata_cli import  hifa_exportdata_cli as hifa_exportdata
from hifa_flagdata_cli import  hifa_flagdata_cli as hifa_flagdata
from hifa_flagtargets_cli import  hifa_flagtargets_cli as hifa_flagtargets
from hifa_fluxcalflag_cli import  hifa_fluxcalflag_cli as hifa_fluxcalflag
from hifa_fluxdb_cli import  hifa_fluxdb_cli as hifa_fluxdb
from hifa_gaincalsnr_cli import  hifa_gaincalsnr_cli as hifa_gaincalsnr
from hifa_gfluxscale_cli import  hifa_gfluxscale_cli as hifa_gfluxscale
from hifa_gfluxscaleflag_cli import  hifa_gfluxscaleflag_cli as hifa_gfluxscaleflag
from hifa_imageprecheck_cli import  hifa_imageprecheck_cli as hifa_imageprecheck
from hifa_importdata_cli import  hifa_importdata_cli as hifa_importdata
from hifa_linpolcal_cli import  hifa_linpolcal_cli as hifa_linpolcal
from hifa_restoredata_cli import  hifa_restoredata_cli as hifa_restoredata
from hifa_session_bandpass_cli import  hifa_session_bandpass_cli as hifa_session_bandpass
from hifa_spwphaseup_cli import  hifa_spwphaseup_cli as hifa_spwphaseup
from hifa_timegaincal_cli import  hifa_timegaincal_cli as hifa_timegaincal
from hifa_tsysflag_cli import  hifa_tsysflag_cli as hifa_tsysflag
from hifa_wvrgcal_cli import  hifa_wvrgcal_cli as hifa_wvrgcal
from hifa_wvrgcalflag_cli import  hifa_wvrgcalflag_cli as hifa_wvrgcalflag
