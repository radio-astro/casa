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
import task_hif_makecutoutimages
def hif_makecutoutimages(vis=[''], offsetblc=[], offsettrc=[], pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Base makecutoutimages task
The hif_makecutoutimages task

Keyword arguments:

---- pipeline parameter arguments which can be set in any pipeline mode

vis -- List of visibility data files. These may be ASDMs, tar files of ASDMs,
   MSs, or tar files of MSs, If ASDM files are specified, they will be
   converted  to MS format.
   default: []
   example: vis=['X227.ms', 'asdms.tar.gz']

offsetblc --  -x and -y offsets to the bottom lower corner (blc) in arcseconds
    default: [0.0,0.0]

offsettrc --  +x and +y offsets to the top right corner (trc) in arcseconds
    default: [0.0,0.0]


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

Output:

results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
   the results object for the pipeline task is returned.


Examples

1. Basic makecutoutimages task

   hif_makecutoutimages()



        """
        if type(vis)==str: vis=[vis]
        if type(offsetblc)==float: offsetblc=[offsetblc]
        if type(offsettrc)==float: offsettrc=[offsettrc]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['vis'] = vis
        mytmp['offsetblc'] = offsetblc
        mytmp['offsettrc'] = offsettrc
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_makecutoutimages.xml')

        casalog.origin('hif_makecutoutimages')
        if trec.has_key('hif_makecutoutimages') and casac.utils().verify(mytmp, trec['hif_makecutoutimages']) :
	    result = task_hif_makecutoutimages.hif_makecutoutimages(vis, offsetblc, offsettrc, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
