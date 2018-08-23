#
# User defined tasks setup.
# Generated from buildmytask.
#

import sys
from casa_stack_manip import stack_frame_find

if sys.path[1] != '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli':
  sys.path.insert(1, '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli')
from odict import odict
if not globals().has_key('mytasks') :
  mytasks = odict()

mytasks['h_applycal'] = 'Apply the calibration(s) to the data'
mytasks['h_export_calstate'] = 'Save the pipeline calibration state to disk'
mytasks['h_exportdata'] = 'Prepare interferometry data for export'
mytasks['h_import_calstate'] = 'Import a calibration state from disk'
mytasks['h_importdata'] = 'Imports data into the interferometry pipeline'
mytasks['h_init'] = 'Initialise the interferometry pipeline'
mytasks['h_mssplit'] = 'Select data from calibrated MS(s) to form new MS(s) for imaging'
mytasks['h_restoredata'] = 'Restore flagged and calibration interferometry data from a pipeline run'
mytasks['h_resume'] = 'Restore a save pipeline state from disk'
mytasks['h_save'] = 'Save the pipeline state to disk'
mytasks['h_show_calstate'] = 'Show the current pipeline calibration state'
mytasks['h_tsyscal'] = 'Derive a Tsys calibration table'
mytasks['h_weblog'] = 'Open the pipeline weblog in a browser'
mytasks['hpc_h_applycal'] = 'Apply the calibration(s) to the data'

if not globals().has_key('task_location') :
  task_location = odict()

task_location['h_applycal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_export_calstate'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_exportdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_import_calstate'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_importdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_init'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_mssplit'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_restoredata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_resume'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_save'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_show_calstate'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_tsyscal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['h_weblog'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
task_location['hpc_h_applycal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/h/cli'
myglobals = stack_frame_find( )
tasksum = myglobals['tasksum'] 
for key in mytasks.keys() :
  tasksum[key] = mytasks[key]

from h_applycal_cli import  h_applycal_cli as h_applycal
from h_export_calstate_cli import  h_export_calstate_cli as h_export_calstate
from h_exportdata_cli import  h_exportdata_cli as h_exportdata
from h_import_calstate_cli import  h_import_calstate_cli as h_import_calstate
from h_importdata_cli import  h_importdata_cli as h_importdata
from h_init_cli import  h_init_cli as h_init
from h_mssplit_cli import  h_mssplit_cli as h_mssplit
from h_restoredata_cli import  h_restoredata_cli as h_restoredata
from h_resume_cli import  h_resume_cli as h_resume
from h_save_cli import  h_save_cli as h_save
from h_show_calstate_cli import  h_show_calstate_cli as h_show_calstate
from h_tsyscal_cli import  h_tsyscal_cli as h_tsyscal
from h_weblog_cli import  h_weblog_cli as h_weblog
from hpc_h_applycal_cli import  hpc_h_applycal_cli as hpc_h_applycal
