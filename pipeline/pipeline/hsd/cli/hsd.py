#
# User defined tasks setup.
# Generated from buildmytask.
#

import sys
from casa_stack_manip import stack_frame_find

if sys.path[1] != '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli':
  sys.path.insert(1, '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli')
from odict import odict
if not globals().has_key('mytasks') :
  mytasks = odict()

mytasks['hsd_applycal'] = 'Apply the calibration(s) to the data'
mytasks['hsd_baseline'] = 'Detect and validate spectral lines, subtract baseline by masking detected lines'
mytasks['hsd_blflag'] = 'Flag spectra based on predefined criteria of single-dish pipeline'
mytasks['hsd_exportdata'] = 'Prepare singledish data for export'
mytasks['hsd_flagdata'] = 'Do basic flagging of a list of MeasurementSets'
mytasks['hsd_imaging'] = 'Generate single dish images'
mytasks['hsd_importdata'] = 'Imports data into the single dish pipeline'
mytasks['hsd_k2jycal'] = 'Derive Kelvin to Jy calibration tables'
mytasks['hsd_restoredata'] = 'Restore flagged and calibration single-dish data from a pipeline run'
mytasks['hsd_skycal'] = 'Calibrate data'
mytasks['hsd_tsysflag'] = 'Flag deviant system temperature measurements'

if not globals().has_key('task_location') :
  task_location = odict()

task_location['hsd_applycal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_baseline'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_blflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_exportdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_flagdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_imaging'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_importdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_k2jycal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_restoredata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_skycal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
task_location['hsd_tsysflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsd/cli'
myglobals = stack_frame_find( )
tasksum = myglobals['tasksum'] 
for key in mytasks.keys() :
  tasksum[key] = mytasks[key]

from hsd_applycal_cli import  hsd_applycal_cli as hsd_applycal
from hsd_baseline_cli import  hsd_baseline_cli as hsd_baseline
from hsd_blflag_cli import  hsd_blflag_cli as hsd_blflag
from hsd_exportdata_cli import  hsd_exportdata_cli as hsd_exportdata
from hsd_flagdata_cli import  hsd_flagdata_cli as hsd_flagdata
from hsd_imaging_cli import  hsd_imaging_cli as hsd_imaging
from hsd_importdata_cli import  hsd_importdata_cli as hsd_importdata
from hsd_k2jycal_cli import  hsd_k2jycal_cli as hsd_k2jycal
from hsd_restoredata_cli import  hsd_restoredata_cli as hsd_restoredata
from hsd_skycal_cli import  hsd_skycal_cli as hsd_skycal
from hsd_tsysflag_cli import  hsd_tsysflag_cli as hsd_tsysflag
