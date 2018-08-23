#
# User defined tasks setup.
# Generated from buildmytask.
#

import sys
from casa_stack_manip import stack_frame_find

if sys.path[1] != '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli':
  sys.path.insert(1, '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli')
from odict import odict
if not globals().has_key('mytasks') :
  mytasks = odict()

mytasks['hifv_applycals'] = 'Applycals'
mytasks['hifv_checkflag'] = 'Run flagdata in rflag mode'
mytasks['hifv_circfeedpolcal'] = 'Base circfeedpolcal task'
mytasks['hifv_exportdata'] = 'Prepare interferometry data for export'
mytasks['hifv_exportvlassdata'] = 'Base exportvlassdata task'
mytasks['hifv_finalcals'] = 'Finalcals'
mytasks['hifv_flagbaddef'] = 'Flagging of deformatters - amp and phase'
mytasks['hifv_flagcal'] = 'Base flagging task'
mytasks['hifv_flagdata'] = 'Do basic deterministic flagging of a list of MeasurementSets'
mytasks['hifv_fluxboot'] = 'Fluxboot'
mytasks['hifv_fluxboot2'] = 'Fluxboot2'
mytasks['hifv_gaincurves'] = 'Runs gencal in gc mode'
mytasks['hifv_hanning'] = 'Hanning smoothing on a dataset'
mytasks['hifv_importdata'] = 'Imports data into the VLA pipeline'
mytasks['hifv_opcal'] = 'Runs gencal in opac mode'
mytasks['hifv_pbcor'] = 'Base pbcor task'
mytasks['hifv_plotsummary'] = 'End of VLA pipeline plotsummary'
mytasks['hifv_priorcals'] = 'Runs gaincurves, opacities, requantizer gains, antenna position corrections, and tec_maps'
mytasks['hifv_restoredata'] = 'Restore flagged and calibration interferometry data from a pipeline run'
mytasks['hifv_rqcal'] = 'Runs gencal in rq mode'
mytasks['hifv_semiFinalBPdcals'] = 'Runs a second quick calibration to set up for heuristic flagging flagging'
mytasks['hifv_solint'] = 'Determines different solution intervals'
mytasks['hifv_statwt'] = 'Statwt'
mytasks['hifv_swpowcal'] = 'Runs gencal in swpow mode'
mytasks['hifv_syspower'] = 'Base syspower task'
mytasks['hifv_targetflag'] = 'Targetflag'
mytasks['hifv_tecmaps'] = 'Base tecmaps task'
mytasks['hifv_testBPdcals'] = 'Runs initial delay calibration to set up heuristic flagging'
mytasks['hifv_vlasetjy'] = 'Does an initial setjy run on the vis'

if not globals().has_key('task_location') :
  task_location = odict()

task_location['hifv_applycals'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_checkflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_circfeedpolcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_exportdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_exportvlassdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_finalcals'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_flagbaddef'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_flagcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_flagdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_fluxboot'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_fluxboot2'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_gaincurves'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_hanning'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_importdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_opcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_pbcor'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_plotsummary'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_priorcals'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_restoredata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_rqcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_semiFinalBPdcals'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_solint'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_statwt'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_swpowcal'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_syspower'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_targetflag'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_tecmaps'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_testBPdcals'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
task_location['hifv_vlasetjy'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli'
myglobals = stack_frame_find( )
tasksum = myglobals['tasksum'] 
for key in mytasks.keys() :
  tasksum[key] = mytasks[key]

from hifv_applycals_cli import  hifv_applycals_cli as hifv_applycals
from hifv_checkflag_cli import  hifv_checkflag_cli as hifv_checkflag
from hifv_circfeedpolcal_cli import  hifv_circfeedpolcal_cli as hifv_circfeedpolcal
from hifv_exportdata_cli import  hifv_exportdata_cli as hifv_exportdata
from hifv_exportvlassdata_cli import  hifv_exportvlassdata_cli as hifv_exportvlassdata
from hifv_finalcals_cli import  hifv_finalcals_cli as hifv_finalcals
from hifv_flagbaddef_cli import  hifv_flagbaddef_cli as hifv_flagbaddef
from hifv_flagcal_cli import  hifv_flagcal_cli as hifv_flagcal
from hifv_flagdata_cli import  hifv_flagdata_cli as hifv_flagdata
from hifv_fluxboot_cli import  hifv_fluxboot_cli as hifv_fluxboot
from hifv_fluxboot2_cli import  hifv_fluxboot2_cli as hifv_fluxboot2
from hifv_gaincurves_cli import  hifv_gaincurves_cli as hifv_gaincurves
from hifv_hanning_cli import  hifv_hanning_cli as hifv_hanning
from hifv_importdata_cli import  hifv_importdata_cli as hifv_importdata
from hifv_opcal_cli import  hifv_opcal_cli as hifv_opcal
from hifv_pbcor_cli import  hifv_pbcor_cli as hifv_pbcor
from hifv_plotsummary_cli import  hifv_plotsummary_cli as hifv_plotsummary
from hifv_priorcals_cli import  hifv_priorcals_cli as hifv_priorcals
from hifv_restoredata_cli import  hifv_restoredata_cli as hifv_restoredata
from hifv_rqcal_cli import  hifv_rqcal_cli as hifv_rqcal
from hifv_semiFinalBPdcals_cli import  hifv_semiFinalBPdcals_cli as hifv_semiFinalBPdcals
from hifv_solint_cli import  hifv_solint_cli as hifv_solint
from hifv_statwt_cli import  hifv_statwt_cli as hifv_statwt
from hifv_swpowcal_cli import  hifv_swpowcal_cli as hifv_swpowcal
from hifv_syspower_cli import  hifv_syspower_cli as hifv_syspower
from hifv_targetflag_cli import  hifv_targetflag_cli as hifv_targetflag
from hifv_tecmaps_cli import  hifv_tecmaps_cli as hifv_tecmaps
from hifv_testBPdcals_cli import  hifv_testBPdcals_cli as hifv_testBPdcals
from hifv_vlasetjy_cli import  hifv_vlasetjy_cli as hifv_vlasetjy
