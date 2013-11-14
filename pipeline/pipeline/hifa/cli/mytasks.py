#
# User defined tasks setup.
# Generated from buildmytask.
#

if sys.path[1] != '/export/home/skye/pipecode/casa/pipeline/pipeline/hifa/cli':
  sys.path.insert(1, '/export/home/skye/pipecode/casa/pipeline/pipeline/hifa/cli')
from odict import odict
if not globals().has_key('mytasks') :
  mytasks = odict()

mytasks['hifa_flagdata'] = 'Do basic flagging of a list of measurement sets'

if not globals().has_key('task_location') :
  task_location = odict()

task_location['hifa_flagdata'] = '/export/home/skye/pipecode/casa/pipeline/pipeline/hifa/cli'
import inspect
myglobals = sys._getframe(len(inspect.stack())-1).f_globals
tasksum = myglobals['tasksum'] 
for key in mytasks.keys() :
  tasksum[key] = mytasks[key]

from hifa_flagdata_cli import  hifa_flagdata_cli as hifa_flagdata
