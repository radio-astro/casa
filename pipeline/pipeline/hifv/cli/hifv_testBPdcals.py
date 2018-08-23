#
# This file was generated using xslt from its XML file
#
# Copyright 2009, Associated Universities Inc., Washington DC
#
import sys
import os
from  casac import *
import string
from taskinit import casalog
from taskinit import xmlpath
#from taskmanager import tm
import task_hifv_testBPdcals
def hifv_testBPdcals(vis=[''], pipelinemode='automatic', dryrun=False, acceptresults=True, weakbp=False, refantignore=''):

        """Runs initial delay calibration to set up heuristic flagging
The hifv_testBPdcals task does an initial delay calibration to set up
heuristic flagging.

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

vis -- List of visibility data files. These may be ASDMs, tar files of ASDMs,
   MSs, or tar files of MSs, If ASDM files are specified, they will be
   converted  to MS format.
   default: []
   example: vis=['X227.ms', 'asdms.tar.gz']


refantignore -- string list to be ignored as reference antennas.
    default: ''
    Example:  refantignore='ea02,ea03'


pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
   determines the values of all context defined pipeline inputs
   automatically.  In 'interactive' mode the user can set the pipeline
   context defined parameters manually.  In 'getinputs' mode the user
   can check the settings of all pipeline parameters without running
   the task.
   default: 'automatic'.

---- pipeline context defined parameter argument which can be set only in
'interactive mode'


--- pipeline task execution modes

dryrun -- Run the commands (True) or generate the commands to be run but
   do not execute (False).
   default: True

acceptresults -- Add the results of the task to the pipeline context (True) or
   reject them (False).
   default: True

weakbp -- Activate the weak bandpass calibrator heuristics
   default: True

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
   the results object for the pipeline task is returned.


Examples

1. Initial delay calibration to set up heuristic flagging.

   hifv_testBPdcals()



        """
        if type(vis)==str: vis=[vis]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
        mytmp['weakbp'] = weakbp
        mytmp['refantignore'] = refantignore
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hifv/cli/"
	trec = casac.utils().torecord(pathname+'hifv_testBPdcals.xml')

        casalog.origin('hifv_testBPdcals')
        if trec.has_key('hifv_testBPdcals') and casac.utils().verify(mytmp, trec['hifv_testBPdcals']) :
	    result = task_hifv_testBPdcals.hifv_testBPdcals(vis, pipelinemode, dryrun, acceptresults, weakbp, refantignore)

	else :
	  result = False
        return result
