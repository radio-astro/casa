#
# User defined tasks setup.
# Generated from buildmytask.
#

import sys
from casa_stack_manip import stack_frame_find

if sys.path[1] != '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsdn/cli':
  sys.path.insert(1, '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsdn/cli')
from odict import odict
if not globals().has_key('mytasks') :
  mytasks = odict()

mytasks['hsdn_importdata'] = 'Imports Nobeyama data into the single dish pipeline'

if not globals().has_key('task_location') :
  task_location = odict()

task_location['hsdn_importdata'] = '/Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hsdn/cli'
myglobals = stack_frame_find( )
tasksum = myglobals['tasksum'] 
for key in mytasks.keys() :
  tasksum[key] = mytasks[key]

from hsdn_importdata_cli import  hsdn_importdata_cli as hsdn_importdata
